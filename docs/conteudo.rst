Entradas e saídas
-----------------

Se estiver a operar em modo contínuo serão produzidos vários ficheiros de saída.
Ao nome definido para o ficheiro de saída é acrescentado um sufixo numérico incremental.

(incompleto)

Configuração
------------
O programa de medição de níveis sonoros é configurável em compilação e em execução
através de opções na linha de comando ou de ficheiro de configuração.

A configuração por opções na linha de comando prevalece sobre a configuração em ficheiro.

(incompleto)

Configurações de compilação
...........................
As configurações de compilação são realizadas no ficheiro ``config.h``.

Na :numref:`config_segment_block`,
o valor de configuração do segmento -- -- e do bloco --  --,
são respetivamente 1 segundo e 1024 amostras

.. literalinclude:: ../src/config.h
   :language: c
   :linenos:
   :caption: Configuração da duração do segment e dimensãp do bloco
   :name: config_segment_block
   :lines: 99-101

(incompleto)

Ficheiro de configuração
........................

O ficheiro de configuração tem o nome ``sound_meter.config``.

(incompleto)

Opções na linha de comando
..........................

Se a opção -i estiver presente e a opção -o não estiver,
o ficheiro de saída vai ter o nome do ficheiro de entrada acrescentado
a extensão da opção -f (".csv" ou ".json").

(incompleto)

Processamento
-------------

O cálculo dos níveis sonoros é realizado em intervalos de tempo designados por **segmento**.
Esse cálculo é sibdividido em cálculos parciais, realizados sobre blocos de dados,
cuja dimensão é configurável em número de amostras.

A duração do segmento e a dimesão do bloco são configuráveis em compilação.

O número de blocos de um segmento depende da sua dimensão e do ritmo de amostragem.
Por exemplo, se a dimensão do bloco for 1024 amostras
e o ritmo de amostragem for 48000 amostras por segundo,
para um segmento de um segundo, existirão 47 blocos,
sendo a dimensão do último bloco de 9xx amostras.

(incompleto)
