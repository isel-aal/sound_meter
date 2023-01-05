## Configuração
------------
O programa de medição de níveis sonoros é configurável em compilação e em execução através de opções na linha de comando ou de ficheiro de configuração.

A configuração por opções na linha de comando prevalece sobre a configuração em ficheiro.

| Parâmetro | Valor embutido no código | Opção de linha de comando | Ficheiro de configuração  |
| ------------------------ | --------------- | ---------------- | --------------------- | 
| Ficheiro de configuração | sound_meter.conf | -g <filename>   |                       |
| Placa de som             | default         | -d <name>        | input_device          |
| Ficheiro de entrada      |                 | -i <filename>    |                       |
| Ficheiro de saída        | sound_meter     | -o <filename>    |                       |
| Local do                 | data/           |                  | output_path           |
| ficheiro de saída        |                 |                  |                       |
| Formato de saída         | CSV             | -f CSV | JSON    |                       |
| Ritmo de amostragem      | 48000           | -r <value>       | sample_rate           |
| Identificação            | XXXX_NNNN       | -n <id>          | identification        |
| Duração do processamento |                 | -t <seconds>     |                       |
| Calibrar                 |                 | -c <seconds>     | calibration_time      |
| Calibração de referência | 94.0 db         |                  | calibration_reference |
| Duração do segmento      | 1               |                  | segment_duration      |
| Duração do bloco         | 1024            |                  | block_size            |
| Período de registo       | 60              |                  | record_period         |
| Período de ficheiro      | 60*60           |                  | file_period           |
| Memória de cálculo de LAeq | 60\*60\*24    |                  | laeq_time             |

### Definição dos parâmetros de configuração:
Identificação
: Identificação da estação.

Placa de som
: Dispositivo de captura de som no formato da bibliteca ALSA. No formato **hw:0,0**, o primeiro número identifica a *card* o segundo o *device* dentro da *card*.

Ritmo de amostragem
: Ritmo de amostragem em amostras por segundo.

Duração do segmento
: Duração do segmento em segundos.

Dimensão do bloco
: Dimensão do bloco em número de amostras.

Período de registo
: Periodo de registo de dados em número de segmentos.

Período de ficheiro
: Periodo de criação de novo ficheiro de registo em número de segmentos. Deve ser um múltiplo de **record_period**.

Local do ficheiro de saída
: Caminho para a diretoria onde são depositados os ficheiros de registo de resultados.

Memória de cálculo de LAeq
: Duração da memória de cálculo de LAeq em número de segmentos.

Período de calibração
: Periodo de calibração no arranque da aplicação, expressa em número de segmentos. O valor zero significa que não faz calibração inicial -- a aplicação utiliza o valor definido em **calibration_reference**.

Calibração de referência
: Valor de calibração de referência (em db).

### Ficheiro de configuração

O ficheiro de configuração pode ser definido na linha de comando com a opção ``-g``.
Na sua ausência, é usado o nome embutido no código, definido em ``config.h`` sob o símbolo ``CONFIG_CONFIG_FILENAME``. Atualmente ``sound_meter.conf``.

Se o primeiro caráter desta definição for diferente de ``/``, trata-se de um nome ou caminho relativo.
Neste caso, a localização do ficheiro é definida na variável de ambiente ``SOUND_METER_PATH``
ou, na sua ausência, pela diretoria corrente em que a aplicação é lançada.

## Processamento
---
O cálculo dos níveis sonoros é realizado em intervalos de tempo designados por **segmento**. Esse cálculo é sibdividido em cálculos parciais, realizados sobre blocos de dados, cuja dimensão é configurável em número de amostras.

A duração do segmento e a dimesão do bloco são configuráveis no ficheiro de configuração, com as etiquetas ``segment_duration`` e ``block_size``, respetivamente.

O número de blocos de um segmento depende da sua dimensão e do ritmo de amostragem.
Por exemplo, se a dimensão do bloco for 1024 amostras e o ritmo de amostragem for 48000 amostras por segundo,
para um segmento de um segundo, existirão 47 blocos,
sendo a dimensão do último bloco de 896 amostras.

Se a definição do ritmo de amostragem não for explicita, será utilizado o valor embutido no código ou o definido no ficheiro WAVE, se for o caso.

## Entradas e saídas
---
O sinal a processar pode ser obtido de ficheiro em formato WAVE ou de placa de som.
O resultado do processamento -- os níveis sonoros -- são registados em ficheiro, em formato CSV.

### Entrada de sinal
O ficheiro de entrada é definido com a opção de linha de comando ``-i``. Na sua ausência o sinal de entrada é obtido de placa de som. A identificação da placa de som é definida pela opção de linha de comando ``-d`` ou no ficheiro de configuração com a etiqueta **input_device**.

### Saída de resultados
O nome do ficheiro de saída pode ser definido com a opção **-o**. Na ausência desta opção, o nome depende da fonte de sinal. Se for um ficheiro WAVE, o ficheiro de saída terá o mesmo nome do ficheiro de entrada, acrescentado da extenção CSV. Se for a placa de som, periodicamente será criado um novo ficheiro de saída.
O periodo de criação do ficheiro de saída é definido no ficheiro de configuração com a etiqueta **file_period**. Os nomes destes ficheiros são formados pelo dia e hora do momento em que são criados. Por exemplo, ``20221231074155.csv`` será o nome do ficheiro criado a 31/12/2022 às 7:41:55.

## Instalação
---
### Geração da aplicação

Esta aplicação utiliza a biblioteca ``libwave``. A variável de ambiente ``PKG_CONFIG_PATH`` deve ser definida com o caminho para a diretoria onde a ``libwave`` tiver sido instalada.
O repositório do projeto tem a seguinte estrutura:
```
sound_meter
   |--- build
   |--- data
   |--- sound_samples
   |--- src
   |--- tests
```
Para gerar o programa executável deve-se invocar ``make`` na diretoria raíz. O executável é depositado na diretoria build com o nome ``sound_meter``.

### Configuração do servidor Web

Servidor Web instalado: NGINX.
Ficheiro de configuração: ``/etc/nginx/sites-available/default``.
Para se ter acesso a todos os ficheiros de uma diretoria acrescentar nesse ficheiro:
```
location /data/ {
	root /home/pi/work/sound/sound_meter/;
	autoindex on;
}
```
Neste caso o conteúdo da diretoria ``/home/pi/work/sound/sound_meter/data`` será mostrado e o seu conteúdo pode ser descarregado. Se a máquina tiver o endereço ``raspberrypi.local`` o URL a especificar no Browser será ``raspberrypi.local/data``.

### Configuração *coredump*

No ficheiro ``/etc/security/limits.conf`` acrescentar: ::
```
pi		soft	core		unlimited
pi		hard	core		unlimited
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




