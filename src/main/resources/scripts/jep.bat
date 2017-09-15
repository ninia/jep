SETLOCAL
SET VIRTUAL_ENV={virtual_env}

IF NOT "%VIRTUAL_ENV%"=="" (
SET PATH="%VIRTUAL_ENV%\bin";"%JAVA_HOME%\bin";"%PATH%"
SET PYTHONHOME=%VIRTUAL_ENV%
)

SET cp="{install_lib}jep\jep-{version}.jar"
IF DEFINED CLASSPATH (
SET cp=%cp%;%CLASSPATH%
)

SET jni_path="{install_lib}jep"

SET args=%*
IF "%args%"=="" (
SET args="{install_lib}jep\console.py"
)

java -classpath %cp% -Djava.library.path=%jni_path% jep.Run %args%
ENDLOCAL
