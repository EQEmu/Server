# EQEmulator Core Server
|Travis CI (Linux)|Appveyor (Windows)   |
|:---:|:---:| 
|[![Linux CI](https://travis-ci.org/EQEmu/Server.svg?branch=master)](https://travis-ci.org/EQEmu/Server)   |[![Windows CI](https://ci.appveyor.com/api/projects/status/d0cvokm7u732v8vl/branch/master?svg=true)](https://ci.appveyor.com/project/KimLS/server/branch/master)   |

***

**EQEmulator is a custom completely from-scratch open source server implementation for EverQuest built mostly on C++**
 * MySQL/MariaDB is used as the database engine (over 200+ tables)
 * Perl and LUA are both supported scripting languages for NPC/Player/Quest oriented events
 * Open source database (Project EQ) has content up to expansion GoD (included in server installs)
  * Game server environments and databases can be heavily customized to create all new experiences
 * Hundreds of Quests/events created and maintained by Project EQ

## Server Installs
||Windows|Linux|
|:---:|:---:|:---:|
|**Install Count**|![Windows Install Count](http://analytics.akkadius.com/?install_count&windows_count)|![Linux Install Count](http://analytics.akkadius.com/?install_count&linux_count)| 
### > Windows 
* [Easy Install](http://wiki.eqemulator.org/p?Akkas_PEQ_Server_Installer&frm=Main#from-scratch-installation-instructions-windows)
* [Advanced Setup](http://wiki.eqemulator.org/p?Complete_Windows-based_Server_Setup_Guide)

### > Debian/Ubuntu

> wget --no-check-certificate https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/linux_installer/install.sh -O install.sh && chmod 755 install.sh && ./install.sh

### > CentOS/Fedora

> curl -O https://raw.githubusercontent.com/EQEmu/Server/master/utils/scripts/linux_installer/install.sh install.sh && chmod 755 install.sh && ./install.sh

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

Resources
---
- [EQEmulator Forums](http://www.eqemulator.org/forums)
- [EQEmulator Wiki](http://wiki.eqemulator.org/i?M=Wiki)
