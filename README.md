# EQEmulator Core Server
|Travis CI (Linux)|Appveyor w/ Bots (Windows)   |Appveyor w/o Bots (Windows)   |
|:---:|:---:|:---:| 
|[![Linux CI](https://travis-ci.org/EQEmu/Server.svg?branch=master)](https://travis-ci.org/EQEmu/Server)   |[![Build status](https://ci.appveyor.com/api/projects/status/scr25kmntx36c1ub/branch/master?svg=true)](https://ci.appveyor.com/project/KimLS/server-87crp/branch/master)   |[![Build status](https://ci.appveyor.com/api/projects/status/mdwbr4o9l6mxqofj/branch/master?svg=true)](https://ci.appveyor.com/project/KimLS/server-w0pq2/branch/master)   |

***

**EQEmulator is a custom completely from-scratch open source server implementation for EverQuest built mostly on C++**
 * MySQL/MariaDB is used as the database engine (over 200+ tables)
 * Perl and LUA are both supported scripting languages for NPC/Player/Quest oriented events
 * Open source database (Project EQ) has content up to expansion OoW (included in server installs)
  * Game server environments and databases can be heavily customized to create all new experiences
 * Hundreds of Quests/events created and maintained by Project EQ

## Server Installs
| |Windows|Linux|
|:---:|:---:|:---:|
|**Install Count**|![Windows Install Count](http://analytics.akkadius.com/?install_count&windows_count)|![Linux Install Count](http://analytics.akkadius.com/?install_count&linux_count)| 
### > Windows 
* [Install](https://github.com/EQEmu/Server/wiki/Windows-Server)

### > Debian/Ubuntu/CentOS/Fedora
* You can use curl or wget to kick off the installer (whichever your OS has)
> curl -O https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/linux_installer/install.sh install.sh && chmod 755 install.sh && ./install.sh

> wget --no-check-certificate https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/linux_installer/install.sh -O install.sh && chmod 755 install.sh && ./install.sh

## Supported Clients

|Titanium Edition|Secrets of Faydwer|Seeds of Destruction|Underfoot|Rain of Fear|
|:---:|:---:|:---:|:---:|:---:|
|<img src="http://i.imgur.com/hrwDxoM.jpg" height="150">|<img src="http://i.imgur.com/cRDW5tn.png" height="150">|<img src="http://i.imgur.com/V48kuVn.jpg" height="150">|<img src="http://i.imgur.com/IJQ0XMa.jpg" height="150">|<img src="http://i.imgur.com/OMpHkKa.png" height="100">|

## Bug Reports <img src="http://i.imgur.com/daf1Vjw.png" height="20">
* Please use the [issue tracker](https://github.com/EQEmu/Server/issues) provided by GitHub to send us bug
reports or feature requests.
* The [EQEmu Forums](http://www.eqemulator.org/forums/) are also a place to submit and get help with bugs.

## Contributions <img src="http://image.flaticon.com/icons/png/512/25/25231.png" width="20">

* The preferred way to contribute is to fork the repo and submit a pull request on
GitHub. If you need help with your changes, you can always post on the forums or
try Discord. You can also post unified diffs (`git diff` should do the trick) on the
[Server Code Submissions](http://www.eqemulator.org/forums/forumdisplay.php?f=669)
forum, although pull requests will be much quicker and easier on all parties.

## Contact <img src="http://gamerescape.com/wp-content/uploads/2015/06/discord.png" height="20">

 - Discord Channel: https://discord.gg/QHsm7CD
 - **User Discord Channel**: `#general`
 - **Developer Discord Channel**: `#eqemucoders`

## Resources
- [EQEmulator Forums](http://www.eqemulator.org/forums)
- [EQEmulator Wiki](https://github.com/EQEmu/Server/wiki)

## Related Repositories
* [ProjectEQ Quests](https://github.com/ProjectEQ/projecteqquests)
* [Maps](https://github.com/Akkadius/EQEmuMaps)
* [Installer Resources](https://github.com/Akkadius/EQEmuInstall)
* [Zone Utilities](https://github.com/EQEmu/zone-utilities) - Various utilities and libraries for parsing, rendering and manipulating EQ Zone files.

## Other License Info

* The server code and utilities are released under **GPLv3**
* We also include some small libraries for convienence that may be under different licensing
  * SocketLib - GPL LibXML
  * zlib - zlib license
  * MariaDB/MySQL - GPL
  * GPL Perl - GPL / ActiveState (under the assumption that this is a free project)
  * CPPUnit - GLP StringUtilities - Apache
  * LUA - MIT
