@echo off
bison -l -d -o sphinxexpryy.cpp sphinxexpr.y
bison -l -d -o sphinxselectyy.cpp sphinxselect.y
bison -l -d -o sphinxqueryyy.cpp sphinxquery.y
