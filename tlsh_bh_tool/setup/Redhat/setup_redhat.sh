#! /usr/bin/env sh

install_prereq() {
    sudo yum install python-pip
    sudo yum install gcc.x86_64
    sudo yum install python-devel
    sudo yum install openssl-devel.x86_64
    sudo yum install gcc-c++
    sudo yum install kernel-devel
}

install_tlsh() {
    # Install TrendMicro [tlsh](https://github.com/trendmicro/tlsh)
    WORK_DIR=tmp

    if [ ! -f ${WORK_DIR}/tlsh.zip ]; then
        echo "downloading tlsh.zip into ${WORK_DIR}/ ..."
        mkdir -p $WORK_DIR
        wget https://github.com/trendmicro/tlsh/archive/master.zip -O $WORK_DIR/tlsh.zip.tmp
        mv $WORK_DIR/tlsh.zip.tmp $WORK_DIR/tlsh.zip
    fi

    unzip -o ${WORK_DIR}/tlsh.zip -d $WORK_DIR

    cd $WORK_DIR/tlsh-master
    ./make.sh
    cd ./py_ext
    python setup.py build
    python setup.py install
    cd ../../../
    rm -rf ${WORK_DIR}/
}

install_anaconda() {
    ANACONDA_DIR=anaconda_dir
    if [ ! -f ${ANACONDA_DIR}/Anaconda2-4.3.0-MacOSX-x86_64.sh ]; then
        echo "downloading anaconda into ${ANACONDA_DIR}/ ..."
        mkdir -p $ANACONDA_DIR
        wget https://repo.continuum.io/archive/Anaconda2-4.3.0-Linux-x86_64.sh -O $ANACONDA_DIR/Anaconda2-4.3.0-MacOSX-x86_64.sh
    fi
    cd $ANACONDA_DIR
    # install Anaconda
    ./${ANACONDA_DIR}/Anaconda2-4.3.0-MacOSX-x86_64.sh 
    # cleanup
    cd ../
    rm -fr ${ANACONDA_DIR}/
    # create anaconda environment
    conda create --name tlsh_tool python=2.7 git -y
    # activate tlsh_tool env
    source activate tlsh_tool
}

install_prereq
install_anaconda
install_tlsh
pip install --user -r requirements.txt
echo "Kindly do a 'source activate tlsh_tool' to activate"
