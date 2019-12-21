#!/bin/bash

cd -P -- "$(dirname -- "${BASH_SOURCE[0]}")"
LD_LIBRARY_PATH=$PWD/lib ./robot