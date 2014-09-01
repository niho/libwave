#create external dir if needed

if [ ! -d "external" ]; then
    echo 'creating external dir'
    mkdir external
fi

cd external
git clone https://github.com/stuffmatic/kowalski.git
git clone https://github.com/tinycthread/tinycthread.git
cd ..