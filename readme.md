## Configuração

O programa de medição de níveis sonoros é configurável em compilação e em execução por meio de:
* opções na linha de comando;
* variáveis de ambiente;
* ficheiro de configuração.

A configuração por opção na linha de comando prevalece sobre a configuração por variável de ambiente ou por ficheiro ficheiro.

O programa pode processar em tempo real som captado por microfone -- designado por **modo contínuo** -- ou processar som gravado em ficheiro no formato WAVE -- designado por **modo discreto**.

| Parâmetro | Valor por omissão (*default*) | Opção de linha de comando | Variável de ambiente | Ficheiro de configuração |
| --------- | --------- | -------------- | ------------- | ----------|
| Ficheiro de configuração | sound_meter_config.json | -g \<filename\> | SOUND_METER_CONFIG_FILENAME | |
| Local do ficheiro de configuração | ./ | | SOUND_METER_CONFIG_FILEPATH | |
| Identificação | XXXX_NNNN | -n \<id\> | | identification |
| Placa de som  | default | -d \<name\> | | input_device |
| Ficheiro de entrada | | -i \<filename\> | | |
| Ficheiro de saída  | sound_meter | -o \<filename\> | | |
| Diretoria para ficheiros de saída | data/ | | | output_path |
| Formato de saída | CSV | -f CSV \| JSON | | output_format |
| Ritmo de amostragem | 48000 | -r \<value\> | | sample_rate  |
| Duração do processamento | | -t \<seconds\> | | |
| Periodo de calibração | | -c \<seconds\> | | |
| Calibração de referência | 94.0 db | | | calibration_reference |
| Duração do segmento | 1 | | | segment_duration |
| Duração do bloco | 1024 | | | block_size |
| Período de registo | 60 | | | record_period |
| Período de ficheiro | 60 * 60 | | | file_period |
| MQTT | false | | | mqtt_enable |
| MQTT broker | tcp://demo.thingsboard.io:1883 | | | mqtt_broker |
| MQTT topic | v1/devices/me/telemetry | | | mqtt_topic |
| MQTT QOS | 1 | | | mqtt_qos |


### Definição dos parâmetros de configuração
Ficheiro de configuração
: Ficheiro com a definição dos parâmetros de configuração em formato JSON.
 
 Local do ficheiro de configuração
 : Local do sistema de ficheiros onde se encontra o ficheiro de configuração.
 
Identificação
: Identificação da estação.

Placa de som
: Dispositivo de captura de som no formato da bibliteca ALSA. No formato **hw:0,0**, o primeiro número identifica a *card* o segundo o *device* dentro da *card*.

Ficheiro de entrada
: O ficheiro de entrada para operação em modo discreto.

Ficheiro de saída
: Ficheiro com os níveis calculados. O formato pode ser CSV ou JSON. Em modo contínuo os nomes dos ficheiros de saída, sucessivamente gerados, são formados pelo dia e hora do momento em que são criados. Por exemplo, ``20221231074155.csv`` será o nome do ficheiro criado a 31/12/2022 às 7:41:55.
 
Em modo discreto se esta opção for omitida o ficheiro de saída terá o mesmo nome do ficheiro de entrada terminado com a a extensão do formato escolhido. Por exemplo:
```
$ sound_meter -i TestNoise.wav -f JSON
```
É gerado um ficheiro com o nome ``TestNoise.wav.json`` depositado na diretoria de saída.

Diretoria para ficheiros de saída
: Caminho para a diretoria onde são depositados os ficheiros de registo dos níveis calculados.

Formato de saída
: Formato do ficheiro de saída com os níveis sonoros. Os formatos possíveis são CSV ou JSON.

Ritmo de amostragem
: Ritmo de amostragem em amostras por segundo. Ignorado em modo discreto.

Duração do processamento
: Em modo contínuo pode ser definido um tempo limite de processamento desde o momento de início. O tempo é definido em segundos.

Período de calibração
: Duração do periodo de calibração no arranque da aplicação, expressa em número de segmentos. O valor zero significa que não faz calibração inicial. Neste caso a aplicação utiliza o valor definido no ficheiro de configuração em **Calibração de referência**.

Calibração de referência
: Valor de calibração de referência (em dba).

Duração do segmento
: Duração do segmento em segundos.

Dimensão do bloco
: Dimensão do bloco em número de amostras.

Período de registo
: Período de registo em ficheiro dos níveis calculados. Período definido em número de segmentos.

Período de ficheiro
: Período de criação de novo ficheiro de registo em número de segmentos. Deve ser um múltiplo de **Período de registo**.

Memória de cálculo de LAeq
: Duração da memória de cálculo de LAeq em número de segmentos.

MQTT
: Ativar a publicação de dados por MQTT.

MQTT broker
: Endereço de internet do Broker MQTT

MQTT topic
: Tópico de submissão dos dados.
 
 MQTT QOS
 : Parâmetro QOS do protocolo MQTT.
 
### Ficheiro de configuração

O ficheiro de configuração pode ser definido na linha de comando com a opção ``-g``.
Na sua ausência, é usado o nome definido na variável de ambiente ``SOUND_METER_CONFIG_FILENAME``
Na ausência de ambas as definições anteriores é utilizado o nome embutido no código, definido em ``config.h`` sob o símbolo ``CONFIG_CONFIG_FILENAME``. Atualmente ``sound_meter_config.json``.

Se o primeiro caráter desta definição for diferente de ``/`` ou ``.``, trata-se de um nome ou caminho relativo.
Neste caso, a localização do ficheiro pode ser definida na variável de ambiente ``SOUND_METER_CONFIG_FILEPATH``.
Na ausência desta variável de ambiênte, considera-se a diretoria corrente onde a aplicação é lançada.

## Processamento

O cálculo dos níveis sonoros é realizado em intervalos de tempo designados por **segmento**. Esse cálculo assenta em processamentos parciais, realizados sobre blocos de amostras, cuja dimensão é uma potência de 2.

A duração do segmento e a dimensão do bloco são configuráveis no ficheiro de configuração, com as etiquetas ``segment_duration`` e ``block_size``, respetivamente.

Um segmento não engloba necessariamente um número inteiro de blocos. Pode existir um bloco com uma primeira parte de amostras pertencente a um segmento e segunda parte de amostras pertencente ao bloco seguinte.

## Instalação

### Instalação no Raspberrypi
Utilizando a placa de som **Respeaker**
À data atual 11-1-2023 o *device driver* da placa Respeaker só funciona até à versão 5.10 do Linux. Deve-se instalar a versão lagacy do Raspi OS.
Para instalar o *device driver*, seguir estas instruções: https://github.com/respeaker/seeed-voicecard
ou estas: https://wiki.seeedstudio.com/ReSpeaker_4-Mic_Linear_Array_Kit_for_Raspberry_Pi

### Instalação da aplicação sound_meter

Primeiro instalar a biblioteca ***libwave***.
```
$ sudo apt install libglib2.0-dev
$ git clone https://github.com/isel-aal/libwave.git
$ cd libwave
$ make
$ sudo ./install.sh
```
A variável de ambiente ``PKG_CONFIG_PATH`` deve ser definida com o caminho para a diretoria onde a ``libwave`` tiver sido instalada. O *script* ``install.sh`` instala em ``/usr/local/lib``.
```
$ export PKG_CONFIG_PATH=/usr/local/lib
```
Gerar a aplicação ***souns_meter***
```
$ sudo apt install libasound-dev libjansson-dev
$ git clone https://github.com/isel-aal/sound_meter.git
$ cd sound_meter
$ make
```
Definir o caminho para a biblioteca *libwave*.
```
$ sudo ldconfig /usr/local/lib
```
### Configuração do servidor Web
Instalar:
```
$ sudo apt nginx
```
Ficheiro de configuração: ``/etc/nginx/sites-available/default``.
Para se ter acesso através do Browser, aos ficheiros produzidos, acrescentar nesse ficheiro:
```
location /data/ {
    root /home/pi/sound_meter/;
    autoindex on;
}
```
O caminho para a diretoria onde são depositados deve ser ajustados conforme a configuração da aplicação sound_meter.

Neste caso o conteúdo da diretoria ``/home/pi/sound_meter/data`` será mostrado e o seu conteúdo pode ser descarregado. Se a máquina tiver o endereço ``raspberrypi.local`` o URL a especificar no Browser será ``raspberrypi.local/data/``.

### Configuração wi-fi 
A interface wireless é configurada no ficheiro ``$ cat /etc/wpa_supplicant/wpa_supplicant.conf``.
```
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=PT

network={
        ssid="AA-AS/TSS4-EI"
        psk="xxxxxxxxx"
}

network={
        ssid="eduroam"
        scan_ssid=1
        key_mgmt=WPA-EAP
        eap=PEAP
        phase1="peaplabel=0"
        phase2="auth=MSCHAPV2"
        identity="ezeq@cc.isel.ipl.pt"
        password="xxxxxxxxx"
}
```
O exemplo contém a configuração para duas redes. A primeira, uma rede doméstica com segurança simples. A segunda, a rede **eduroam**, com auteticação de utilizador.

##### Situação particular relativa à rede **eduroam**.

À data desta instalação, 16 de janeiro 2023, a ligação à *eduroam* apresentava erro.
Para contornar esta dificuldade foram seguidas as instruções descritas na conversa:
``https://forums.raspberrypi.com/viewtopic.php?t=253567``

### Acesso remoto

Para acesso ao sistema como servidor, é conveniente ter uma referência fixa. O endereço IP não é solução porque muda ao longo do tempo. A solução normal é a utilização de DNS.

A solução adotada foi a utilização de Dynamic DNS através do sítio NO-IP. A utilização deste sítio engloba a criação de uma conta em www.noip.com e a utilização de uma aplicação cliente para atualização do endereço IP  (Dynamic Update Cliente - DUC).
Por omissão, a aplicação cliente atualiza o servidor DNS com o endereço público do *router*. Isso permite a acesso de qualquer ponto da Internet mas implica configurar o router com *port forwarding*. Na rede local do ISEL está fora de questão realizar *port forwarding*.
A aplicação DUC permite definir o endereço a comunicar ao servidor DNS. Configurando com um endereço local é possível realizar acessos na intranet do ISEL. 

Para instalar a aplicação DUC seguir as instruções desta página: https://my.noip.com/dynamic-dns/duc. 

A opção -i permite definir o endereço IP associado ao *hostname*.
Na página https://my.noip.com/dynamic-dns foi definido o *hostname* ``soundmeter1.ddns.net``.

A aplicação DUC é executada no *script* ``no-ip-update.sh`` que obtêm o endereço IP corrente com o utilitário ``ipaddress``.
```
#!/bin/bash

address=`ipaddress wlan0`
noip2 -c ~/.no-ip/no-ip2.conf -i $address
```

### Configuração *coredump*

No ficheiro ``/etc/security/limits.conf`` acrescentar: ::
```
pi      soft    core        unlimited
pi      hard    core        unlimited
```
Depois de fazer *reboot* verificar com:
```
$ ulimit -c
unlimited
```
No ficheiro ``/proc/sys/kernel/core_pattern`` definir o padrão do nome do ficheiro core. (Consultar ``$ man core``.)
```
$ sudo bash -c 'echo core.%e.%t > /proc/sys/kernel/core_pattern
```

#### Analisar o *core dumped*
Lançar o GDB.
```
$ gdb sound_meter core.xxxxx.yyyyy
```
Depois pode-se usar comandos do gdb.
Por exemplo: backtrace; up; down.
