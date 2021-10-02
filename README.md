# QP-7C
## QP-7Cミニ送信機の改造（その1）

7010 kHz以外の使用と多バンド化対応に向けて改造しました．  
改造は以下の2点です．  
### 1．Si5351モジュールを使用した任意周波数での出力  
	Si5351モジュール（秋月電子）を使用し，発振出力”CLK0”を68pFのコンデンサーを介してドライブ段に入力しました．  
	Si5351は，Arduino Nano互換機からI2Cインターフェースで制御．  
	秋月電子のSi5351モジュールは3.3V駆動なので，本来はI2Cバスの電圧変換(Arduino Nanoは5V)が必要ですが，アマチュア的に電流制限抵抗(10 kΩ)の直列接続で使用します．  
	周波数変更の最も単純なやり方は，Arduinoのプログラムで周波数を固定して発振させる（周波数を変えたいときはプログラムを変える）です．  
	（LCD, OLEDやロータリーエンコーダーを付ければ，周波数を見ながら単独で周波数変化も可能となります．）  
送信出力は，xxxxxxでした．  
### 2．多バンド化に向けて，ドライブ段とファイナル段の結合トランスのソケット化およびLPFのソケット化  
	トランス：8ピンICソケットの上にDIY7を乗せてハンダ付け，受け側も8ピンICソケットに  
		他のバンド用には、例えばFCZコイル（サトー電気）+共振コンデンサーが使用可能か？  
	LPF：8ピンICソケットの上に基板を乗せてその上にLPFを構成，受け側も8ピンICソケットに  
		80m帯、30m帯、20mバンド用のLPFにはCRK-10Aの2バンド化改造やD4Dの回路図などを参考にしてLPFソケット上に作成可能と思われます．  
	他バンドはまだ試していません．  

具体的には，図のようにChocolate基板（3x2）上に作製しました．  
Arduino Nano互換機のアナログ端子側(図の下側，+3.3V, +5V, Vin端子のある側)には，Chocolate基板上にピンソケットを付けて，Arduino Nano互換機と接続します．  
ただし，RSTピンはピンソケットを曲げるか切るかして接続しません（そのまま繋ぐとグランドに接続されてしまうので）．  
デジタル端子側(図の上側, D0-D12がある側)はChocolate基板にはピンソケットを付けず，Arduino Nano互換機のピンは空中に浮かしておきます．  
（デジタル端子側のGNDピンだけは，ピンソケットを付けても良い（付けると力学的に安定化します））．  
この図の配置ですと，+5V出力ピンがChocolate基板のグランドに当たってしまうので，図の赤線に沿ってChocolate基板をコの字型にパターンカットします．  
Si5351へ電源供給はArduino Nano互換機の+3.3V出力ピンから行います．  
Arduino Nano互換機のUSB端子から電源を受け，+5V出力ピンおよび+3.3V出力ピンへ電源出力しますので，ここからドライブ段とファイナル段への5V供給も可能です．  
Chocolate基板のグランドは全体に繋がっているので，電源の配線は＋側だけでも大丈夫です．  
DCジャック使用時は，Arduino Nano互換機のVin端子に電源供給します．  
（ブレッドボード用ＤＣジャックＤＩＰ化キットを使えば，DCジャックをChocolate基板上に配置できます．）  
Si5351の制御（I2C）にはA4ピン(SDA), A5ピン(SCL)を使用します．  
Si5351モジュール，トランス，LPF用に8ピンICソケットを付けました．  
トランスとLPFをソケット化したので，部品配置がオリジナルとは少し変わっています．  
トランスの8ピンICソケットへの取り付けは写真のようにしました．  
電鍵用のジャックも，基板取付用３.５ｍｍステレオミニジャックを使えば，Chocolate基板上に配置できます．  
図中の赤いジャンパー線は基盤の裏側（銅箔側），黒いジャンパー線は部品側で配線です．  
太いジャンパー線(+3.3VとI2Cの配線)は，基盤の裏側で被覆線を使って接続です．  
LPFもChocolate基板（1x1）上に作製できます（図を参照）．  
Chocolate基板のカットは，カッターナイフで切り込みを入れてから手で割れば可能です．  
銅箔側に部品を置いた方が，8ピンICソケットの半田付けが楽です．  
#### 最も単純なArduino Nano用スケッチは，”QP-7C_simple.ico”です．  
  
#### 必要追加部品(2021年10月1日時点の価格)  
	Chocolate基板：CRkits共同購入プロジェクト，380円　http://jl1kra.sakura.ne.jp/chocolate.html  
	Arduino Nano互換機：500円から1,000円程度，オリジナルは高価  
	秋月電子Si5351モジュール：秋月電子，500円　https://akizukidenshi.com/catalog/g/gK-10679/  
	8Pソケット（Si5351モジュール受け、DIY7、DIY7受け、LPFソケット受け）：秋月電子，15円x4　https://akizukidenshi.com/catalog/g/gP-00035/  
	8P連結ソケット（両端オスピン）（LPFソケット用）：秋月電子，55円　https://akizukidenshi.com/catalog/g/gP-00264/  
	ブレッドボード用ＤＣジャックＤＩＰ化キット：秋月電子，100円　https://akizukidenshi.com/catalog/g/gK-05148/  
	基板取付用３．５ｍｍステレオミニジャック：秋月電子，50円　https://akizukidenshi.com/catalog/g/gC-12478/  
	分割ロングピンソケット　１×４２（Arduino Nano受け）：秋月電子，80円　https://akizukidenshi.com/catalog/g/gC-05779/  
	ピンヘッダ　１×４０（電源？）：秋月電子，35円　https://akizukidenshi.com/catalog/g/gC-00167/  
	電解コンデンサ 16V, 100μF（電源用）：2個  
	積層コンデンサ10μF(Si5351電源用）：1個  
	積層コンデンサー1 uF(Nano電源用）：1個  
	抵抗　10kΩ(Si5351モジュールI2C電圧降下用）：2個  





## QP-7Cミニ送信機の改造（その2）  
  
QP-7Cミニ送信機の改造（その1）に続いて，デジタルモード(FT8)送信を可能にします．  
オリジナル情報は”Ein FT8-QRP-Transceiver” https://www.elektronik-labor.de/HF/FT8QRP.html です．　  
ドイツ語なのでGoogle翻訳やDeepL翻訳を使って読みましょう．  
基本原理は，以下の通りです．  
多くのデジタルモード(FT8など)では，いくつかの周波数を切り替えて送信しています．  
例えば，FT8では8つの周波数を6.25baud(1秒間に6.25回の頻度)で変化させています．  
一般には，この信号をPCからのオーディオ信号で入力してSSB変調しています（D4D改や13TR-FT8など）．  
”Ein FT8-QRP-Transceiver”では，まずPCからのオーディオ信号をArduinoで受け、その周波数を測定します．  
次に，Si5351(PLL発振機)で、基準周波数+オーディオ周波数の信号を発振させ、増幅し、送信します．  
デジタル変調によって、オーディオ信号の周波数が変われば、それに追随して、発振周波数を変化させます．  
SSBモードを利用した通常のマイク入力のデジタル変調はAFSK方式ですが、この方式はFSKになっています．  
直接、信号周波数を発振させているので、SSBモードのAFSK方式で問題となってしまう搬送波の漏れやサイドバンドの漏れは原理的に起こりません．  
一般のデジタルモードの信号周波数変化(変調速度)は比較的遅いので、uSDXでのSSB発生に比べても処理は簡単です．  
（ちなみにuSDXでのSSB発生の原理も，この方法と基本的には似ています．）  
  
具体的には，QP-7Cミニ送信機の改造（その1）に追加します．  
### 1．PCからのオーディオ入力用ジャックとLPFの設置  
	”Ein FT8-QRP-Transceiver”と同じくArduino Nano互換機のD7ピンをコンパレーターとして使用し、D6ピンをグランドに接続します．  
	オリジナル”Ein FT8-QRP-Transceiver”では、カットオフ周波数1.6 kHzですが、もっと高くしても良いのでは（例えば10 kΩを4.7 kΩ、10 nFを4.7 nFにすると、7.2 kHzとなる)．  
	オーディオ信号の１サイクルの時間を計測して周波数を算出しています．  
	Arduino Nano互換機の下に，オーディオ入力用ジャックとオーディオLPFを配置することが可能です（図を参照のこと）．  
	Arduino Nanoのプログラム（スケッチ）はhttps://www.elektronik-labor.de/HF/FT8QRP.html にオリジナル版とCAT control support版があります．  
	プログラムに少し問題点があるようなので，少し修正しました．  
### 2．CW、デジタルモードの切替を行えるようにしました．  
	電鍵端子の電圧をチェックし、Arduino Nano互換機の電源オン時に電鍵端子に押されていない電鍵が接続されていればCWモード、そうでなければデジタルモードとしました．  
  
#### Arduino Nano用スケッチは”QP-7C_FT8.ico”．  
  
#### 必要追加部品   
	基板取付用３．５ｍｍステレオミニジャック   
	積層コンデンサ1μF(オーディオ入力用）：1個   
	積層コンデンサ4.7nF(オーディオ入力用）：2個   
	抵抗　1MΩ(オーディオ入力用）：1個   
	抵抗　4.7kΩ(オーディオ入力用）：3個(1個はQP-7Cミニ送信機キットの余りを流用可）   




## QP-7Cミニ送信機の改造（その3）   
   
Si5351は複数の発振出力があり、受信用にも使用可能です．  
QP-7Cミニ送信機の改造（その1）あるいはQP-7Cミニ送信機の改造（その2）に続いて，Simple Mini Receiverキットを使用してトランシーバー化しました．  
   
Simple Mini Receiverキットをほぼそのままで利用しますが，このキットではグラウンド接続を必要としないイヤフォン出力となっているので，そのままではPCへの接続ができません．  
このため、D4Dの回路と同様に2N3904のコレクタと電源の間に抵抗(1kΩ程度)を接続して，コレクタからコンデンサー10uFを通して出力をとることにしました．  
（ダイナミックイヤフォンを接続しても音量が小さいので，その場合は追加アンプを付けるかセラミックイヤフォンを使用する必要があります．）  
（CA3028 Rxキットを使えばオーディオアンプが付いているので，PCでもダイナミックイヤフォンでもスピーカーでも大丈夫でしょう．)  
Si5351のCLK1を68pFのコンデンサーを介してSA612の6番ピンに入力します（水晶発振子は使用しない）．  
QP-7Cミニ送信機の改造で余ったDIY7を使用します（多バンド対応のためソケット化しました）．  
将来的にRFプリアンプを追加できるようにソケットに5V電源を供給可としました．  
入力段のフィルターX’talと直列Cは差替え可能にするため，ソケット化しました．  
  
送受切替は，リレー切替(D4D方式，JR6JGB方式など)がベストと思われますが，信機側の入力インピーダンスを高くして直結(CRK-10A方式)もありそうです．  
今回、QP-7Cミニ送信機の改造で余ったDIY7を受信機に使いたいという理由と，QP-7C送信機に後付けで受信部を付けたいという理由から，
送信時に受信部をアンテナ回路から切断し、受信時に接続する方式(MOSFETスイッチ(QCX(QRP Lab.)方式)にしました．  
汎用MOSFET（BS170）を使用したオンオフスイッチで、MOSFETスイッチの制御はArduino Nanoから行います．  
CW運用時には、Arduino Nanoで電鍵のオンオフ状態を検出します（D8ピン）．  
デジタルモード運用時には、オーディオ信号の有無で検出します（VOX動作）．  
これらをChocolate基板上に配置したのが図です．  
#### Arduino Nano用スケッチは"QP-7C_FT8_tranceiver.ico"です．  
  
#### 必要追加部品(2021年10月1日時点の価格)  
	Simple Mini Receiverキット：CRkits共同購入プロジェクト，1020円　http://jl1kra.sakura.ne.jp/SimpleRX.html  
	8Pソケット（DIY7ソケット用）：秋月電子，15円x2　https://akizukidenshi.com/catalog/g/gP-00035/  
	基板取付用３．５ｍｍステレオミニジャック：秋月電子，50円　https://akizukidenshi.com/catalog/g/gC-12478/  
	1N4148(Rx入力保護)：秋月電子，100円(50本入）　https://akizukidenshi.com/catalog/g/gI-00941/  
	BS170(Tx-Rx切替)：秋月電子，20円 https://akizukidenshi.com/catalog/g/gI-09724/  
	丸ピンＩＣ用ソケット（X’tal, C差替え用）：秋月電子,150円　https://akizukidenshi.com/catalog/g/gP-01591/  
	コネクタ付コード　４Ｐ　（赤黒黄緑)：秋月電子，80円 https://akizukidenshi.com/catalog/g/gC-15385/  
	コンデンサ10μF(オーディオ出力用）：1個  
	コンデンサ0.1μF（Tx-Rx切替）：1個  
	コンデンサ102（Tx-Rx切替）：1個  
	コンデンサ103（Tx-Rx切替）：1個(QP-7Cミニ送信機キットの余りを流用可）  
	抵抗　1kΩ(オーディオ出力用）：1個 (QP-7Cミニ送信機キットの余りを流用可）  
  
#### 最終的に余る部品：  
	QP-7Cミニ送信機キット：2N4401, 発振用X’tal, 100pFx2, 1kΩ, 22kΩ, 基板　(DIY7-7, 103, 1kΩ, 4.7kΩを流用可）  
	Simple Mini Receiverキット：LED, 発振用X’tal, 39pF, 1μF, SMAソケット  
