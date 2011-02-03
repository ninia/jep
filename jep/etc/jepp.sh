export LD_PRELOAD=/usr/`rpm --eval %_lib`/libpython2.6.so
export CLASSPATH=$CLASSPATH:${exec_prefix}/lib/jepp/jepp-2.4.jar
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${exec_prefix}/lib/jepp
