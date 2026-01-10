#!/bin/bash

licznik=1
ls -1 zips/ | while read -r line; do
  ((licznik++))
  unzip zips/$line -d "unpacked/pack_${licznik}"
done
