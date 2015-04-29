SET VIRTUAL_ENV={virtual_env}

{ld_library_path}

IF NOT "%VIRTUAL_ENV%"=="" (
SET PATH="$VIRTUAL_ENV\bin;%PATH%"
SET PYTHONHOME="$VIRTUAL_ENV"
)

SET cp="{prefix}\lib\jep\jep.jar"
IF DEFINED CLASSPATH (
SET cp="%cp%;%CLASSPATH%"
)

SET args=%*
IF "%args%"=="" (
SET args="{install_lib}jep\console.py"
)

java -classpath %cp% jep.Run %args%