@echo off
javac -cp . -d . *.java
jar cfm sphinxapi.jar MANIFEST.MF org/sphx/api
