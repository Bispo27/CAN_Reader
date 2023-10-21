# CAN_Reader

O objetivo desse projeto é um leitor de rede CAN usando um sensor de efeito Hall. A mensagem CAN trafegando no barramento gera um campo magnético, 
que é lido pelo sensor de efeito hall e é convertido em um sinal binário (0 ou 1).
Esse código é feito para um ESP32, e ele trata os bits, montando um Frame de mensagem CAN válido.
Após montar o Frame, ele gera um Access Point, que podemos nos conectar via Wi-Fi.
Ao acessar o AP e entrar no navegador no IP 3.3.3.3, é mostrado uma página oude podemos configurar alguns parâmetros da CAN (bitrate e etc...) 
Conforme as leituras vão sendo feitas e os frames vão sendo montados, os valores da CAN são mostratos no navegador em uma caixa de texto.
Além do dado visual, também é possível armazenar as informações em um arquivo logsCan.log 
