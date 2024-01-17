#!/bin/bash

url="http://ccfit.nsu.ru/~rzheutskiy/test_files/50mb.dat"

make_request() {
    local request_url=$1
    curl --http1.0 -i -x 127.0.0.1:90 "$request_url"
}

make_request $url &
make_request $url

wait

echo "Both requests completed."
