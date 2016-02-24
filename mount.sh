#!/bin/bash

cd pcmapi && make && cd ..

sudo mount linux-0.2.img 1/
sudo cp pcmapi/ptest 1/bin
sudo umount 1/

