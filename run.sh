./build.sh
bs code/main.bsh
irc res/main.ir -o res/main
./res/main write hello
echo "Returned with" $?
