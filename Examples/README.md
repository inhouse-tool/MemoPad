# Examples

## 動作確認用テキストファイル

### 1024byte目以降にShiftJISがあってもUTF8と自動判定される件.txt

このテキストファイルをダウンロードして, Windows11の“メモ帳”( Notepad.exe ) で開くと,
最後の方が文字化けして見えると思います. ( 2023年11月現在 )
この不具合は
[Windows10 に対して指摘された方がいらっしゃる](https://www.reddit.com/r/Windows10/comments/ntourv/i_found_a_bug_with_notepad_encoding_autodetection/)のですが,
いまだに修正される気配がありません.

そう, このご指摘は「Windows10 に対して」なんです.
「ウチの Windows10 ではそんな文字化けにはならんな」と思った方,
Double click や Drag & Drop でファイルを開くのではなく,
ファイルメニューの「開く」からファイルを開いてみてください.
ほうら! 化けたでしょ!
さっきの[ご指摘](https://www.reddit.com/r/Windows10/comments/ntourv/i_found_a_bug_with_notepad_encoding_autodetection/)をよーく読むと,
ちゃんとそう書いてあるんですって.

というわけで, 古い話なんです, この文字化けって.
それがいまだに直されていません.

まあ, こんな状況なので, 「メモ帳アプリぐらい自分で作った方が早いかな」と思った次第です.
「メモ帳アプリなんて [CFrameWnd](https://learn.microsoft.com/en-us/cpp/mfc/reference/cframewnd-class) に
[CEdit](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) 貼り付けるだけで出来るし」
とタカをくくって…….
( ※個人の [MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library) 使用者の発想です。)

### CEditもWin11NotepadもすべてのUnicodeが表示できるわけではない件.txt

で, アプリを作ってみて動かしてみると, 一部の Unicode が正しく表示されませんでした.
[Unicode 一覧](https://ja.wikipedia.org/wiki/Unicode一覧_10000-10FFF) に基づいて,
サンプリングで確認してみたときに使ったのがこのテキストファイルです.

「まあ, [CEdit](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) の
Unicode 対応もそんなもんかな」と思いつつ, 「Windows11 の Notepad.exe はどうなんだろ?」
と比べてみると, 意外にも大した違いはありませんでした.
むしろ「[CEdit](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) では表示されるのに
 Notepad.exe では表示されない字」があるのは驚きでした.
「大した違いがないなら, これでいいや」と結論付けた次第です.

おや? このファイル, `Microsoft Print to PDF` で印刷すると,
.pdf ファイルのサイズが 0 になりますね.

調べてみると, どうやら UNICODE の 1f0e0 ～ 1f0f5 の文字が地雷のようです.
[Unicode 一覧](https://ja.wikipedia.org/wiki/Unicode一覧_1F000-1FFFF) によると,
カード ( タロットカード? ) を表す文字コードの領域ですね.
しかし, NG な文字だけが出ないとか化けるのならともかく,
.pdf ファイル全体がやられるとは…….
( `Microsoft XPS Document Writer` ではそんなことにはならないので,
`Microsoft Print to PDF` 固有の現象のようです. )

### ところで……

こんなイヤガラセのような長いファイル名を付けると,
Notepad のタブには表示しきれませんね.
「タブじゃなくて新しいウィンドウで」と設定しても,
ファイル名を表示するスペースがタブのままってのはどうなんでしょう?

## 表示例

### MemoPadInTheDark.png

ダークモードにおける表示例. なんか全体的に暗いです.

### MemoPadInTheLight.png

ライトモードにおける表示例. なんか全体が白っぽいです.

### いずれにせよ……

色使いが微妙ですが, ダークモード対応アプリをいくつか調べたところ,
こんな感じの表現が主流なようですのよ.

<p style="text-align: right;">
In-house Tool / 家中 徹
</p>
