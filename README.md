# MemoPad

ただの「メモ帳」です.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="Examples/MemoPadInTheDark.png">
  <source media="(prefers-color-scheme: light)" srcset="Examples/MemoPadInTheLight.png">
  <img alt="Snapshot" src="Examples/MemoPadInTheLight.png">
</picture>

<p>

[MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library)
業界界隈では「初心者の練習問題」扱いされるほど, 簡単に作れるものです.
なぜそんなものをいまさら持ち出したのかというと,
Windows11&reg; でリニューアルされた
[Notepad](https://apps.microsoft.com/detail/windows-notepad/9MSMLRH6LZF3?hl=ja-jp&gl=jp)
で目立つようになった文字化けが動機です.

この文字化けの原因は

* Notepad が文字のエンコードを「自動検出」する際に, ファイルの冒頭 1,024 バイトしか見ていないから

のようです.
この 1,024 バイト中に Shift JIS 文字が含まれて**いなければ**,
( Shift JIS に代わってデフォルトに躍り出た )
UTF-8 と判定し,
1,024 バイト以降に挟まっていた Shift JIS 文字も UTF-8 でデコードしようとして,
**化けた文字を表示**することになります.

というわけなので,
最初から Shift JIS が並んでいるテキストファイルでは文字化けは見られず,
冒頭から 1,024 バイト以上の英数字が並んだ後にようやく Shift JIS が登場するようなファイルに限って,
文字化けすることになります.
「なんかたまに化けるんだけど……」というモヤモヤした感触は, この発症メカニズムから来るものでしょう.

* Shift JIS な日本語の混ざった .html ファイル
* Shift JIS な日本語の注釈が挟まったプログラムソースファイル
* Mail header 込みで受信ログを Shift JIS 保存したファイル ( DKIM-Signature があると1,024越え確実 )

などは, 文字化け遭遇率が高まるでしょうね.

この文字化けの回避策としてネット上で「提案」されているものと, その「感想」は

| 案 | 感想 |
| --- | --- |
| 開く際に「自動検出」から「ANSI」に指定しなおす | 次を開くたびに「自動検出」に戻るのでしんどい |
| Windows10&reg; の PC から Notepad.exe を移植する | ファイルメニューから開くと同じ現象に遭遇する |
| レジストリをごにょごにょする | 利かなかった…… |
| もっとちゃんとしたエディターを使うことにする | そんなに大げさなものを使うほどの話でも…… |

といったところでしょうか. ( ※個人の感想です。)

で, 原因が判っている人間として
「も少しマシな自動検出のアルゴリズム」をついうっかり思いついてしまったので,
界隈の人間の体質で「練習問題」だと思って作ってしまったわけです.

## 概要

* それはもうびっくりするぐらい面白味のない, ただの「メモ帳」です.
* おおむね昔ながらの「メモ帳」程度の機能が搭載されています.
* Windows11&reg; の Notepad と異なり, 音声入力には非対応です. しかし, その分軽く仕上がっております.
* Windows11&reg; の Notepad と異なり, タブにも非対応です. ここで目指しているのは昔の「あのメモ帳」です.
* こっそりネットにつないでどこかに通信するような機能は入っておりません.
* というか, アヤしい機能は一切入っておりません. ウイルスチェックに引っかかったとしたら誤検出です.
* 誤検出の場合, rebuild すれば収まるようです. Hash 値かなんかが変わってパターンから外れるからかな?
* 何度 rebuild しても誤検出が収まらない場合, それはあなたが追加したコードのせいです.

## 環境

* Visual Studio 2022&reg; での Windows11 向け build なので, 64bit OS 用です.
* 同じ build で Windows10 でも問題なく動作しますが, 64bit 版に限ります.
* Windows7? 何を言っているんですか? 絶海の孤島で Windows7 を崇めている 7教の信者か何かですか?
* Windows8? 何それ? おいしいの?

## 運用

 [Releases](https://github.com/inhouse-tool/MemoPad/releases)
にインストーラー `MemoPad.msi` を用意しました.
まずはこれで[インストール](https://github.com/inhouse-tool/MemoPad/tree/master/Installation.md)しましょう.
その後, 必要に応じて [DLL の追加](MemoPad/Release/README.md) を行うことになります.
( これまでの PC の使い方によっては, DLL の追加 は不要な場合もあります. )

「.txt [ファイルとの関連付け](https://www.google.com/search?q=ファイルとの関連付け)」
はご利用者自らやっていただく前提です.
どんな種類のファイルを「メモ帳」で開きたいのかは, 人それぞれですから.
ただし, 関連付けを行わなくとも, `File` メニューの `Open` からファイルを指定したり,
あらかじめ起動しておいた本アプリに開きたいファイルを Drag & Drop すれば,
お好きなファイルを開くことができます.
「ファイルとの関連付け」は,
File Explorer 上でファイルをダブルクリックしたときに,
このアプリで開くために必要な儀式です.

運用をやめ, 本アプリをアンインストールする際は,
Windows の `設定 ( Settings )` から `アプリ ( Apps )` に進んで,
`インストールされているアプリ ( Installed apps )` の中から
`MemoPad` を探して,
その行の右端にある`…` をクリックして `アンインストール ( Uninstall )` を選んでください.
その後ファイルをダブルクリックすると,
アプリとの関連を見失った File Explorer が「どのアプリで開く？」と訊いてきますが,
そのときに元祖 Notepad を指定し直せばいいだけのことです.
なんのリスクもありません.

そうそう, リスクと言えば, お定まりの免責事項を添えておきましょうか.

> 当方は、本品の運用において生じた事故について、一切の責任を負わないものとします。


## License

```
MIT License

Copyright (c) 2023 In-house Tool

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

なに言ってんだ? えーと, つまり
「タダで使っていいよ」ってことだな, これは.<br>
でも「使ってなんかあっても責めないでね」とも言ってるな.
まー, タダなんだし, そんなもんか.

## 特徴

いまさら「メモ帳」についてご説明申し上げるのもナンなので,
以下 Windows10 の元祖メモ帳 ( 以下「元祖10」) と Windows11 の本家メモ帳 ( 以下「元祖11」) との違いを列挙しておきます.

### 全体

* 元祖10とは異なり, Dark mode にお付き合いします. この点に関しては元祖11側の立場です.
* 両元祖と異なり, 出てくるダイアログに `Cancel` ボタンがありません.
ダイアログ右上の`×`を押してキャンセルしていただくデザインです.
`Esc` キーを押すのでもいいです.
元祖11でも一部そういうデザインになっていますね. 一部であって統一されているわけではありませんが.
* 両元祖と異なり, 「Shift JIS」 を 「ANSI」 の中に含めていません.
ASCII コードのみのテキストは「ASCII」と, Shift JIS が一文字でも混入したテキストは「Shift JIS」と表示します.
「ANSI」という混乱を招く表現は避けました.
そもそも Japanese Industrial Standards なものを American National Standards Institute
に包含した元祖の表現が紛らわしいと思うのです. この際なので言わせていただければ<sub>【以下検閲により削除】</sub>
* 両元祖とはステータスバーに表示される情報の順序が異なります.
こじんまりとまとまるように入れ替えました.
* メニューをたどっている間, ステータスバーの左側にメニューの説明文が出ます.
両元祖とは異なりますが, [MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library)
での製造物はこれがデフォルトの芸風です. なんか親切そうに見えるので, こちらの芸風を採用しました.
* 両元祖と異なり, `Insert` キーを押すとカーソルの形状が縦棒とブロックの間で切り替わります.
作業の中断などでカーソルを見失った際は,
`Insert` キーを二度押しすると, どこで中断したか見つけやすいでしょう.
* 両元祖と異なり, 起動するとプライマリーモニターの左側に**ほぼ**目いっぱいの高さで出現します.
当初, **きっちり**目いっぱいの高さで出していたのですが,
Windows の余計なお世話にひっかかってウザいことになると判明したので,
底を 1pixel だけ浮かせてあります.
なお, なぜ右側に寄せなかったのかというと,
アプリを閉じるとき, 勢いあまって背後にいる最大化した別アプリまで閉じてしまうというリスクを避けるためです.
* 両元祖と同じく, `Ctrl` キーを押しながらマウスのホイールをグリグリすると, 表示の倍率が変わります.
この操作方法について何の説明もしていないのも, 両元祖と同じです.
Windows 業界の常識だから?
でも, File Explorer で`Ctrl`＋グリグリすると, ちょっと違う動きをするしなあ…….

### File メニュー

* 両元祖と異なり, `File` メニューの `Print` は「プリンターの選択」と「用紙の選択」と「ページの選択」が,
まとめて一緒のダイアログに表示されます.
「A4 に出そうと思ってたのに A3 に出ちゃった」といった事故を減らすために,
印刷開始の `OK` ボタンと同じところに用紙の選択も見えるようにしました.
* 上記ダイアログの用紙サイズの選択は, mm 単位モノと inch 単位モノで別のリストが出ます.
元祖ではまとめて同じリストに出ますが, 「そんな用紙、こっちじゃ売ってねえよ。」という気がしたので改めました.
* いずれにせよ用紙のサイズ指定は,
Windows の指定候補にあるものを並べて一方的に「コレで頼む」と指示しているだけです.
A4 しか対応していないプリンターにも「A3 で頼む」とか操作できちゃいますので,
ご使用のプリンターの仕様に合わせてご指定ください.
* 上記ダイアログの中に, 余白を設定するためのボタン `Margins` があります.
「ページ設定」 ( `Page setup` ) が `File` メニューにないのは, こんなところにあるからです.
* 印刷時のヘッダーやフッターは余白の中央に印刷されます.
元祖のフッターやヘッダーは本文にくっついて印刷されるのですが,
ヘッダーなのか本文の一番上の行なのか見分けがつかないので改めました.
* [ヘッダーやフッターの「コマンド」](https://support.microsoft.com/ja-jp/windows/メモ帳でヘッダーとフッターのコマンドを変更する-c1b0e27b-497d-c478-c4c1-0da491cac148)に,
「&b」というのを勝手に加えました. 本文との間に境界線を引きます.
より一層ヘッダー感やフッター感が増します. (当社比)
* 両元祖同様,
「印刷のプレビュー」といった機能は備わっていないので,
`Microsoft Print to PDF` でも相手に練習して印刷イメージに納得してから,
実際に紙に出すのがよろしいかと.
* 両元祖と異なり, `File` メニューの `Properties` から今開いているファイルの概要が見られます.
ステータスバーの表示をはしょったユーザーに向けての救済策みたいなもんです.

### Edit メニュー

* 元祖10と異なり Undo ( `Ctrl`+`Z` ) の重ねがけができます. この点に関しては元祖11と同じです.
`Ctrl`+`Z` を二度押しすると, 元祖10では「やっぱやめるのやめた」で元に戻るのに対し,
「やっぱやめてその前のもやめた」となり, いじる前に向けて 2段階戻ります.
* メニューにはありませんが, `Ctrl`+`Y` で Redo となるのは元祖11同様です.
メニューに出していないところも元祖11と同じです.
`Ctrl`+`Z` を二度押しして 「やっぱやめてその前のもやめた」 のあと,
`Ctrl`+`Y` を一度押すと 「やっぱその前のはやめない」 となり, いじる前に向けて 1段階だけ戻った状態に進みます.
* 開いたテキストファイルを少しでもいじるとタイトルバーに出ているファイル名の前に
「いじったけどまだセーブしてないよ」を知らせる * マークが付きます.
Undo ( `Ctrl`+`Z` ) で元に戻すと * マークは消えます.
そう表現するのが自然だと思うのですが……
元祖11だとアプリ画面のタブ上のファイル名表示には反映されませんね.
でも, 23H2 以降でタスクバーにアプリのタイトルが出るようにしておくと,
タスクバー上の表示では同様の表現がなされています.
元祖10では一度付いた * マークは Undo ( `Ctrl`+`Z` ) しても消えることはありません.
* 元祖11と異なり, `Find` や `Replace` に使うダイアログは, アプリ画面の外に出せます.
この点は元祖10と同じです.
アプリ画面の中に居座る元祖11のダイアログ(?)ってものすごく邪魔なような気がしますし.
* 両元祖と異なり, `Edit` メニューの `Replace` の `All` は, 一括ですべてを置き換えるのではなく,
パタパタと置き換えを繰り返します.
実装上の都合 (「一括で」という処理の作りこみをサボったから ) ですが,
見ていてなんか楽しいです.
* 両元祖と異なり,  `Replace` の `All`  後の Undo ( `Ctrl`+`Z` ) は一気にすべてを戻すのではなく,
やっぱりパタパタと元に戻していきます.
おおらかな気持ちで見守ってやってください.

### View メニュー

* 両元祖と異なり, `View` メニューの `Menu bar` でメニューバーも消せます. 1行でも多く表示したいときにどうぞ.
* メニューバーを消してしまった後, 元に戻せなくて困った場合は, タイトルバーを右クリックしてください.
元に戻すためのメニューにたどり着けます.
* 両元祖と異なり, `View` メニューの `Font...` からフォントを選ぶようになっています.
* 両元祖と異なり, フォントを選ぶにはまず Character set を選んでどこの国の文字を表示したいかを絞り,
プロポーショナルフォントと等幅フォントのいずれか ( あるいは両方か ) も選んでから,
フォント名を選ぶようにしてあります.
元祖のようにいきなり「フォント名で選べ」と全部出されても, 候補数が多くてしんどいので.
* ここでフォントが Character set で分類されているのが, 混乱を招きますね.
「“Shift JIS” ってのがあるけど UTF-8 でも使えるのかな?」
とか考えてしまいます.
おまけに“ANSI”ってのもあって,
元祖の区分だと“ANSI”は“Shift JIS”を包含しているはずなので
「“ANSI”のフォントなら“Shift JIS”な文字も表現できるのでは?」
と思わせてしまいます.
そもそも<sub>【検閲により削除】</sub>が<sub>【検閲により削除】</sub>を<sub>【検閲により削除】</sub>するから<sub>【検閲により削除】</sub>.
……というわけで,
**日本語を扱うなら“Shift JIS”に属するフォントを選んでおけば,
全文きれいに表示されます.**
なお, この Character set に限って“ANSI”との表記を変えなかったのは,
ここで独自表現を加えるとややこしい話がさらにややこしくなるからです.
致し方ありません. 所詮<sub>【以下検閲により削除】</sub>
* 両元祖と異なり, フォントを選ぶとすかさず画面の表示に反映されます.
プレビューとして見ながらフォントを選ぶのに便利です.
(「謝罪文に丸ゴシックのフォントだと、なんか反省の色が感じられないなあ。」とか. )
* フォントを選んでから `OK` ボタンを押すと, それが記録され次の起動時のデフォルトフォントになります.
フォントを入れ替えて遊んでいるだけの場合は, `OK` を押さずに右上の`×`を押してダイアログを消すのがいいでしょう.
その場合でも, プレビューとしての画面は最後に遊んだフォントのまま残ります.
本来はここでデフォルトに戻すべきとのご意見もあるようですが, 総合的かつ俯瞰的に判断した結果, 遊び優先となりました.

### Help メニュー

* 両元祖同様, `Help` に自前の説明文を実装したりせず, サイトにつないで~~ごまかして~~容量の節約に努めています.
* 両元祖とは異なり, `About` に表示されるバージョンは 3組の数値です. 最新バージョンは 1.0.5 です.


<sup>
※上に挙げた元祖の振る舞いは 2023年11月現在のものです.
これらの振る舞いはユーザーにお断りなく変更される場合が割とちょくちょくあります.
</sup>

## 今後

今後についてですが, 現状以上の機能追加は計画しておりません.
これ以上何か加えると「あのメモ帳」から逸脱するので.
ただし, バグフィックス等の保守作業は今後とも継続する所存です.
保守しかしないので「保守派」です.
Windows11 Notepad のような革新派とは路線が異なります.
新機能の追加とかは, ココとは別口でお進めください.
その下敷きとしてココのソースが少しでもお役に立てるのであれば,
どうぞご自由にお持ちください.

## 謝辞

* ネット上で情報を公開してくださっているみなさまのお陰で, このソフトは出来ました. 感謝申し上げます.
* このソースは Visual Studio&reg; で作成されたものなので, その版元の Microsoft&reg; さんにも感謝しときます.

<div style="text-align: right;">
In-house Tool / 家中 徹
</div>
