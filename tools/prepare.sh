#!/bin/sh
cp header.py build.py
cat ../libjingle-0.6.14/talk/libjingle.scons >> ./build.py
echo "This is building file list:"
python build.py $1
rm build.py *.pyc
