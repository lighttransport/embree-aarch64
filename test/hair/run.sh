#!/bin/bash

if [ "$#" -gt "0" ]; then
  echo $1
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"$1"
fi

./test_hair
