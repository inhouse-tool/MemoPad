# Examples

## ����m�F�p�e�L�X�g�t�@�C��

### 1024byte�ڈȍ~��ShiftJIS�������Ă�UTF8�Ǝ������肳��錏.txt

���̃e�L�X�g�t�@�C�����_�E�����[�h����, Windows11�́g�������h( Notepad.exe ) �ŊJ����,
�Ō�̕��������������Č�����Ǝv���܂�. ( 2023�N11������ )
���̕s���
[Windows10 �ɑ΂��Ďw�E���ꂽ��������������](https://www.reddit.com/r/Windows10/comments/ntourv/i_found_a_bug_with_notepad_encoding_autodetection/)�̂ł���,
���܂��ɏC�������C�z������܂���.

����, ���̂��w�E�́uWindows10 �ɑ΂��āv�Ȃ�ł�.
�u�E�`�� Windows10 �ł͂���ȕ��������ɂ͂Ȃ��ȁv�Ǝv������,
Double click �� Drag & Drop �Ńt�@�C�����J���̂ł͂Ȃ�,
�t�@�C�����j���[�́u�J���v����t�@�C�����J���Ă݂Ă�������.
�ق���! �������ł���!
��������[���w�E](https://www.reddit.com/r/Windows10/comments/ntourv/i_found_a_bug_with_notepad_encoding_autodetection/)����[���ǂނ�,
�����Ƃ��������Ă����ł�����.

�Ƃ����킯��, �Â��b�Ȃ�ł�, ���̕�����������.
���ꂪ���܂��ɒ�����Ă��܂���.

�܂�, ����ȏ󋵂Ȃ̂�, �u�������A�v�����炢�����ō���������������ȁv�Ǝv��������ł�.
�u�������A�v���Ȃ�� [CFrameWnd](https://learn.microsoft.com/en-us/cpp/mfc/reference/cframewnd-class) ��
[CEdit](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) �\��t���邾���ŏo���邵�v
�ƃ^�J���������āc�c.
( ���l�� [MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library) �g�p�҂̔��z�ł��B)

### CEdit��Win11Notepad�����ׂĂ�Unicode���\���ł���킯�ł͂Ȃ���.txt

��, �A�v��������Ă݂ē������Ă݂��, �ꕔ�� Unicode ���������\������܂���ł���.
[Unicode �ꗗ](https://ja.wikipedia.org/wiki/Unicode�ꗗ_10000-10FFF) �Ɋ�Â���,
�T���v�����O�Ŋm�F���Ă݂��Ƃ��Ɏg�����̂����̃e�L�X�g�t�@�C���ł�.

�u�܂�, [CEdit](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) ��
Unicode �Ή�������Ȃ��񂩂ȁv�Ǝv����, �uWindows11 �� Notepad.exe �͂ǂ��Ȃ񂾂�?�v
�Ɣ�ׂĂ݂��, �ӊO�ɂ��債���Ⴂ�͂���܂���ł���.
�ނ���u[CEdit](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) �ł͕\�������̂�
 Notepad.exe �ł͕\������Ȃ����v������̂͋����ł���.
�u�債���Ⴂ���Ȃ��Ȃ�, ����ł�����v�ƌ��_�t��������ł�.

����? ���̃t�@�C��, `Microsoft Print to PDF` �ň�������,
.pdf �t�@�C���̃T�C�Y�� 0 �ɂȂ�܂���.

���ׂĂ݂��, �ǂ���� UNICODE �� 1f0e0 �` 1f0f5 �̕������n���̂悤�ł�.
[Unicode �ꗗ](https://ja.wikipedia.org/wiki/Unicode�ꗗ_1F000-1FFFF) �ɂ���,
�J�[�h ( �^���b�g�J�[�h? ) ��\�������R�[�h�̗̈�ł���.
������, NG �ȕ����������o�Ȃ��Ƃ�������̂Ȃ�Ƃ�����,
.pdf �t�@�C���S�̂������Ƃ́c�c.
( `Microsoft XPS Document Writer` �ł͂���Ȃ��Ƃɂ͂Ȃ�Ȃ��̂�,
`Microsoft Print to PDF` �ŗL�̌��ۂ̂悤�ł�. )

### �Ƃ���Łc�c

����ȃC���K���Z�̂悤�Ȓ����t�@�C������t�����,
Notepad �̃^�u�ɂ͕\��������܂����.
�u�^�u����Ȃ��ĐV�����E�B���h�E�Łv�Ɛݒ肵�Ă�,
�t�@�C������\������X�y�[�X���^�u�̂܂܂��Ă̂͂ǂ��Ȃ�ł��傤?

## �\����

### MemoPadInTheDark.png

�_�[�N���[�h�ɂ�����\����. �Ȃ񂩑S�̓I�ɈÂ��ł�.

### MemoPadInTheLight.png

���C�g���[�h�ɂ�����\����. �Ȃ񂩑S�̂������ۂ��ł�.

### ������ɂ���c�c

�F�g���������ł���, �_�[�N���[�h�Ή��A�v�������������ׂ��Ƃ���,
����Ȋ����̕\�����嗬�Ȃ悤�ł��̂�.

<p style="text-align: right;">
In-house Tool / �ƒ� �O
</p>
