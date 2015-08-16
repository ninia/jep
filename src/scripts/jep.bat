SET VIRTUAL_ENV={virtual_env}

:: TODO For vitualenv, we should really include the old PATH in the redefined
:: PATH, but after hours of trying to get it right but having quotes mess it
:: up, I give up. If you put quotes around it such as
:: SET PATH="VIRTUAL_ENV%\bin;%PATH%"
:: it can't find java. But without quotes if there is anything in the PATH
:: with a space, that'll also break it. Need help from a Windows expert.
IF NOT "%VIRTUAL_ENV%"=="" (
SET PATH=%VIRTUAL_ENV%\bin;%JAVA_HOME%\bin
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
