#create external dir if needed

if [ ! -d "external" ]; then
    echo 'creating external dir'
    mkdir external
fi

cd external
git clone https://github.com/stuffmatic/kowalski.git
git clone https://github.com/tinycthread/tinycthread.git
git clone https://github.com/akheron/jansson.git
git clone https://git.xiph.org/opus.git
cd opus
git checkout tags/v1.1
cd ..
cd ..