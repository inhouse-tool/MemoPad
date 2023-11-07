# MemoPad

ただの「メモ帳」です.

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
| もっとちゃんとしたエディタを使うことにする | そんなに大げさなものを使うほどの話でも…… |

といったところでしょうか. ( ※あくまで個人の感想です。)

で, 原因が判っている人間として
「も少しマシな自動検出のアルゴリズム」をついうっかり思いついてしまったので,
界隈の人間の体質で「練習問題」だと思って作ってしまったわけです.

## 機能

* それはもうびっくりするぐらい面白味のない, ただの「メモ帳」です.
* おおむね昔ながらの「メモ帳」程度の機能が搭載されています.
* ただし, 印刷関連の機能は搭載しておりません. ~~なんかめんどくさかったから~~ SDGs 的観点からの判断です.
* Windows11&reg; の Notepad と異なり, 音声入力には非対応です. しかし, その分軽く仕上がっております.
* ネットにつないでどこかに通信するような機能は入っておりません.
* というか, アヤしい機能は一切入っておりません. ウイルスチェックに引っかかったとしたら誤検出です.
* 誤検出の場合, rebuild すれば収まるようです. Hash 値かなんかが変わってパターンから外れるからかな?
* 何度 rebuild しても誤検出が収まらない場合, それはあなたが追加したコードのせいです.

## 環境

* Visual Studio 2022&reg; での Windows11 向け build なので, 64bit OS 用です.
* 同じ build で Windows10 でも問題なく動作しますが, 64bit 版に限ります.
* Windows7? 何を言っているんですか? 絶海の孤島で Windows7 を崇めている 7教の信者か何かですか?
* Windows8? 何それ? おいしいの?

## 運用

ここではインストーラーなどを提供しておりません. 単品の .exe だけを build するためのセットです.
この .exe はお使いの PC のどこのフォルダに置いてもかまいません.
( お作法としては C:\Program Files\ の下に置くべきなんでしょうけど. )

「.txt [ファイルとの関連付け](https://www.google.com/search?q=ファイルとの関連付け)」とか
「[ショートカットの作成](https://www.google.com/search?q=ショートカットの作成)」はご利用者自らやっていただく前提です.
その気になれば
「[ショートカットをスタートメニューに登録](https://www.google.com/search?q=ショートカットをスタートメニューに登録)」
することも可能なはずです.

運用をやめ, すべてをなかったことにするには, .exe を削除してください.

設定を変えるとレジストリに記録が残ります.
これもなかったことにするには [regedit](https://www.google.com/search?q=regedit) で
```
\HKEY_CURRENT_USER\Software\In-house Tool
```
を丸ごと削除してください. ほっといても今後に何も差し障りありませんが.

あと, 上記の説明で「ちょっと何言ってるか解んない」な方は, レジストリの削除はやめておいた方がいいでしょう.
リスクの割には得るところがないので.

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

## 謝辞

* ネット上で情報を公開してくださっているみなさまのお陰で, このソフトは出来ました. 感謝申し上げます.
* このソースは Visual Studio&reg; で作成されたものなので, その版元の Microsoft&reg; さんにも感謝しときます.

<div style="text-align: right;">
In-house Tool / 家中 徹
</div>