NAME := eqemu-server
.ONESHELL:

DOCKER_ARGS := --rm  --name ${NAME} -v $$PWD:/src -w /src ${NAME}
DOCKER_ARM64_ARGS := --rm --platform linux/arm64 --name ${NAME}-arm64 -v $$PWD:/src -w /src ${NAME}-arm64

.PHONY: build
build:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile build --no-print-directory
	exit
endif
	cd build$$BUILD_SUFFIX && cmake --build . --config Release --target all --

.PHONY: cmake
cmake:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile cmake --no-print-directory
	exit
endif
	@echo "working directory: $$PWD"
	mkdir -p build$$BUILD_SUFFIX
	@cd build$$BUILD_SUFFIX && cmake -DEQEMU_BUILD_LOGIN=ON \
			-DEQEMU_BUILD_TESTS=ON \
			-DCMAKE_CXX_COMPILER_LAUNCHER=ccache -G Ninja ..

clean:
ifneq (,$(findstring .devcontainer,$$PWD))
	@make -C ../ -f .devcontainer/Makefile clean --no-print-directory
endif
	rm -rf build

docker-cmake:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile docker-cmake --no-print-directory
	exit
endif
	@echo "working directory: $$PWD"
	git submodule update --init --recursive
	docker run ${DOCKER_ARGS} make cmake

docker-build:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile docker-build --no-print-directory
	exit
endif
	docker run ${DOCKER_ARGS} make build

# Build image if it doesn't exist
docker-image-build:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile docker-image-build --no-print-directory
	exit
endif
ifeq ($(shell docker images -q ${NAME} 2> /dev/null),)
	@echo "Docker image not found. Building..."
	docker build -f Dockerfile.debian.dev -t ${NAME} .
endif

docker-arm-cmake: docker-arm-image-build
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile docker-arm-cmake --no-print-directory
	exit
endif
	git submodule update --init --recursive
	docker run ${DOCKER_ARM64_ARGS} make cmake BUILD_SUFFIX=arm64

docker-arm-build: docker-arm-image-build
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile docker-arm-build --no-print-directory
	exit
endif
	docker run ${DOCKER_ARM64_ARGS} make build BUILD_SUFFIX=arm64

docker-arm-image-build:
ifeq ($(shell docker images -q ${NAME}-arm64 2> /dev/null),)
	@echo "Docker image not found. Building..."
	docker build -f Dockerfile.debian.arm.dev -t ${NAME}-arm64 .
endif

docker-clean: clean

.PHONY: prep
prep:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile prep --no-print-directory
	exit
endif
	@echo "Preparing build/bin for usage..."
	mkdir -p build/bin/assets/patches
	cp -R -u -p .devcontainer/base/eqemu_config.json build/bin/eqemu_config.json
	cp -R -u -p .devcontainer/base/login.json build/bin/login.json
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

quests:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile quests --no-print-directory
	exit
endif
	@cd build/bin && git clone https://github.com/ProjectEQ/projecteqquests.git quests

# Runs tests
.PHONY: test
test:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile test --no-print-directory
	exit
endif
	cd build/bin && ./tests

# Runs login binary
.PHONY: login
login:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile login --no-print-directory
	exit
endif
	cd build/bin && ./loginserver

# Runs shared_memory binary
.PHONY: shared
shared:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile shared --no-print-directory
	exit
endif
	cd build/bin && ./shared_memory

# Runs zone binary
.PHONY: zone
zone:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile zone --no-print-directory
	exit
endif
	@-rm build/bin/logs/zone/zone*.log
	cd build/bin && ./zone

# Runs world binary
.PHONY: world
world:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile world --no-print-directory
	exit
endif
	@-rm build/bin/logs/world*.log
	cd build/bin && ./world

# Runs ucs binary
.PHONY: ucs
ucs:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile ucs --no-print-directory
	exit
endif
	@-rm build/bin/logs/ucs*.log
	cd build/bin && ./ucs

# Runs queryserv binary
.PHONY: queryserv
queryserv:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile queryserv --no-print-directory
	exit
endif
	@-rm build/bin/logs/query_server*.log
	cd build/bin && ./queryserv

valgrind-%:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile valgrind --no-print-directory
	exit
endif
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
	-cd base/db && unzip db.sql.zip
endif
	@echo "Sourcing db may take a while, please wait..."
	@cd base/db/peq-dump && sudo mariadb --database peq -e "source create_all_tables.sql"
	@echo "MariaDB is now injected."

.PHONY: gm-%
gm-%:
	sudo mariadb --database peq -e "UPDATE account SET status=255 WHERE name = '$*';"
	@echo "Account $* is now a GM. /camp to have it go into effect."

depends:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile depends --no-print-directory
	exit
endif
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
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile backup --no-print-directory
	exit
endif
	@mkdir -p build/bin/backup
	cd build/bin && ./world database:dump --compress --player-tables --state-tables --system-tables --query-serv-tables

cpu-zone:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile cpu-zone --no-print-directory
	exit
endif
	@cd build/bin && mkdir -p tmp
	cd build/bin && CPUPROFILE=prof.out ./zone

pprof-zone:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile pprof-zone --no-print-directory
	exit
endif
	cd build/bin && google-pprof --pdf zone prof.out > prof.pdf
pprof-web-zone:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile pprof-web-zone --no-print-directory
	exit
endif
	cd build/bin && google-pprof --web zone prof.out
pprof-gv-zone:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile pprof-gv-zone --no-print-directory
	exit
endif
	cd build/bin && google-pprof --gv zone prof.out > prof.gv
heap-zone:
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@make -C ../ -f .devcontainer/Makefile heap-zone --no-print-directory
	exit
endif
	@cd build/bin && mkdir -p tmp
	cd build/bin && HEAPPROFILE=prof.out ./zone
