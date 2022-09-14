# optee_vuln
A vuln demo on TA，inclding overflow, type confusion and toctou.
### 1.安装optee依赖库

sudo apt-get install android-tools-adb android-tools-fastboot autoconf \ 
     automake bc bison build-essential ccache codespell cpio \ 
     cscope curl device-tree-compiler expect flex ftp-upload gdisk iasl \ 
     libattr1-dev libcap-dev libcap-ng-dev \ 
     libfdt-dev libftdi-dev libglib2.0-dev libgmp-dev libhidapi-dev \ 
     libmpc-dev libncurses5-dev libpixman-1-dev libssl-dev libtool make \ 
     mtools netcat ninja-build python-crypto python3-crypto python-pyelftools \ 
     python3-pycryptodome python3-pyelftools python3-serial \ 
     rsync unzip uuid-dev xdg-utils xterm xz-utils zlib1g-dev



### 2.获取repo

mkdir -p ~/.bin 
 PATH="${HOME}/.bin:${PATH}" 
 curl https://storage.googleapis.com/git-repo-downloads/repo > ~/.bin/repo 
 chmod a+rx ~/.bin/repo



### 3.获取源代码

mkdir optee-qemu && cd optee-qemu 
 repo init -u https://github.com/OP-TEE/manifest.git -m qemu_v8.xml 
 repo sync -j4 --no-clone-bundle

### 4.获取工具链

optee-qemu文件夹的路径

cd <optee-project>/build 
 make -j2 toolchains

### 5.修改op-tee-os的debug等级

optee_os/mk/config.mk的58行改为CFG_TEE_TA_LOG_LEVEL ?= 4

### 6.编译前

三个漏洞demo放在optee_examples/目录下

### 7.编译

build目录下sudo make -f qemu_v8.mk all

### 8.运行

build目录下sudo make -f qemu_v8.mk run-only

### 9.运行

type_confusion

toctou

overflow
