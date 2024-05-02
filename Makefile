NAME := eqemu-server

DOCKER_ARGS := --rm  --name ${NAME} -v ${PWD}:/src -w /src ${NAME}

# git remote add eqemu git@github.com:EQEmu/server.git
# git fetch eqemu
# git checkout -b eqemu-master eqemu/master
# git branch foo/2024-03-18 # create a new branch based on latest main
# git checkout foo/2024-03-18 # switch to the new branch
# git merge eqemu-master foo/2024-03-18 # merge all the changes on my eqemu-master branch to xackery/may-latest
# git remote rm eqemu # remove the eqemu remote
# git branch -d eqemu-master # remove the eqemu-master branch locally

# Run 'ninja' in build directory
.PHONY: build
build:
	@if [ ! -d "build$$BUILD_SUFFIX" ]; then \
		make cmake; \
	fi

	@cd build$$BUILD_SUFFIX && cmake --build . --config Release --target all --

# Run 'cmake' in build directory
.PHONY: cmake
cmake:
	mkdir -p build$$BUILD_SUFFIX
	@cd build$$BUILD_SUFFIX && cmake -DEQEMU_BUILD_LOGIN=ON \
			-DEQEMU_BUILD_TESTS=ON \
			-DCMAKE_CXX_COMPILER_LAUNCHER=ccache -G Ninja ..

clean:
	rm -rf build

# Run 'cmake' in ubuntu docker container
docker-cmake: docker-image-build
	git submodule update --init --recursive
	docker run ${DOCKER_ARGS} make cmake

# Run 'ninja' in ubuntu docker container
docker-build: docker-image-build
	docker run ${DOCKER_ARGS} make build

# Build image if it doesn't exist
docker-image-build:
	@echo "Docker image not found. Building..."
	cd .devcontainer && docker build -f Dockerfile.debian.dev -t ${NAME} .

docker-clean: clean

.PHONY: prep
prep:
	@echo "Preparing build/bin for usage..."
	mkdir -p build/bin/assets/patches
	cp -R -u -p base/eqemu_config.json build/bin/eqemu_config.json
	cp -R -u -p base/login.json build/bin/login.json
	cp -R -u -p loginserver/login_util/* build/bin/assets/patches/
	mkdir -p build/bin/assets
	cp -R -u -p utils/patches build/bin/assets/
	-unlink build/bin/lua_modules
	cd build/bin && ln -s quests/lua_modules lua_modules
	-unlink build/bin/mods
	cd build/bin && ln -s quests/mods mods
	-unlink build/bin/maps
	cd build/bin && ln -s ../../base/maps maps
	mkdir -p build/bin/logs
	mkdir -p build/bin/shared
	@echo "Eqemu is prepared. Edit build/bin/eqemu_config.json to configure."

maps:
	@echo "Downloading maps..."
	@mkdir -p base/maps
	@cd base/maps && wget -nc https://github.com/Akkadius/eqemu-maps/archive/refs/heads/master.zip
	@cd base/maps && unzip -o master.zip
	@cd base/maps && mv eqemu-maps-master/* .
	@cd base/maps && rm -rf eqemu-maps-master
	@echo "Maps downloaded."

# Runs tests
.PHONY: test
test:
	cd build/bin && ./tests

# Runs login binary
.PHONY: login
login:
	cd build/bin && ./loginserver

# Runs shared_memory binary
.PHONY: shared
shared:
	cd build/bin && ./shared_memory

# Runs zone binary
.PHONY: zone
zone:
	@-rm build/bin/logs/zone/zone*.log
	cd build/bin && ./zone

# Runs world binary
.PHONY: world
world:
	@-rm build/bin/logs/world*.log
	cd build/bin && ./world

# Runs ucs binary
.PHONY: ucs
ucs:
	@-rm build/bin/logs/ucs*.log
	cd build/bin && ./ucs

# Runs queryserv binary
.PHONY: queryserv
queryserv:
	@-rm build/bin/logs/query_server*.log
	cd build/bin && ./queryserv

valgrind-%:
	cd build/bin && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=logs/$*.valgrind.log ./$*

# Start mariaDB standalone
.PHONY: mariadb
mariadb:
	@sudo service mariadb start

.PHONY: inject-mariadb
inject-mariadb:
	-sudo service mariadb start
	-mkdir -p base/db/
	-sudo mariadb -e 'DROP DATABASE IF EXISTS peq;'
	-sudo mariadb -e 'CREATE DATABASE peq;'
	-sudo mariadb -e "CREATE USER 'peq'@'127.0.0.1' IDENTIFIED BY 'peqpass';"
	-sudo mariadb -e "GRANT ALL PRIVILEGES ON *.* TO 'peq'@'127.0.0.1';"
ifeq (,$(wildcard base/db/db.sql.zip))
	@echo "base/db.sql.zip not found. Downloading latest from https://db.projecteq.net/"
	wget -nc https://db.projecteq.net/latest -O base/db/db.sql.zip
endif
	-cd base/db && sudo service mariadb start && unzip db.sql.zip
	@echo "Sourcing db may take a while, please wait..."
	@cd base/db/peq-dump && sudo mariadb --database peq -e "source create_all_tables.sql"
	@echo "MariaDB is now injected."

.PHONY: gm-%
gm-%:
	sudo mariadb --database peq -e "UPDATE account SET status=255 WHERE name = '$*';"
	@echo "Account $* is now a GM. /camp to have it go into effect."

depends:
	sudo apt install graphviz pip time
	pip3 install graphviz
	mkdir -p build/depends
	@if [ ! -f "build/depends/dependency_graph.py" ]; then \
		wget https://raw.githubusercontent.com/pvigier/dependency-graph/master/dependency_graph.py -O build/depends/dependency_graph.py; \
	fi
	@echo "Generating dependency graphs (This may take a while)..."
	@echo "Login..."
	time python3 build/depends/dependency_graph.py -f png login build/depends/login.dot
	@echo "World..."
	time python3 build/depends/dependency_graph.py -f png world build/depends/world.dot
	@echo "Zone..."
	time python3 build/depends/dependency_graph.py -f png zone build/depends/zone.dot
	@echo "Common..."
	time python3 build/depends/dependency_graph.py -f png common build/depends/common.dot

backup:
	@mkdir -p build/bin/backup
	cd build/bin && ./world database:dump --compress --player-tables --state-tables --system-tables --query-serv-tables

cpu-zone:
	@cd build/bin && mkdir -p tmp
	cd build/bin && CPUPROFILE=prof.out ./zone

pprof-zone:
	cd build/bin && google-pprof --pdf zone prof.out > prof.pdf
pprof-web-zone:
	cd build/bin && google-pprof --web zone prof.out
pprof-gv-zone:
	cd build/bin && google-pprof --gv zone prof.out > prof.gv
heap-zone:
	@cd build/bin && mkdir -p tmp
	cd build/bin && HEAPPROFILE=prof.out ./zone
