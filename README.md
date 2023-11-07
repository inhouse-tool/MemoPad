# MemoPad

�����́u�������v�ł�.

[MFC](https://en.wikipedia.org/wiki/Microsoft_Foundation_Class_Library)
�ƊE�E�G�ł́u���S�҂̗��K���v���������ق�, �ȒP�ɍ�����̂ł�.
�Ȃ�����Ȃ��̂����܂��玝���o�����̂��Ƃ�����,
Windows11&reg; �Ń��j���[�A�����ꂽ
[Notepad](https://apps.microsoft.com/detail/windows-notepad/9MSMLRH6LZF3?hl=ja-jp&gl=jp)
�Ŗڗ��悤�ɂȂ����������������@�ł�.

���̕��������̌�����

* Notepad �������̃G���R�[�h���u�������o�v����ۂ�, �t�@�C���̖`�� 1,024 �o�C�g�������Ă��Ȃ�����

�̂悤�ł�.
���� 1,024 �o�C�g���� Shift JIS �������܂܂��**���Ȃ����**,
( Shift JIS �ɑ����ăf�t�H���g�ɖ��o�� )
UTF-8 �Ɣ��肵,
1,024 �o�C�g�ȍ~�ɋ��܂��Ă��� Shift JIS ������ UTF-8 �Ńf�R�[�h���悤�Ƃ���,
**������������\��**���邱�ƂɂȂ�܂�.

�Ƃ����킯�Ȃ̂�,
�ŏ����� Shift JIS ������ł���e�L�X�g�t�@�C���ł͕��������͌���ꂸ,
�`������ 1,024 �o�C�g�ȏ�̉p���������񂾌�ɂ悤�₭ Shift JIS ���o�ꂷ��悤�ȃt�@�C���Ɍ�����,
�����������邱�ƂɂȂ�܂�.
�u�Ȃ񂩂��܂ɉ�����񂾂��ǁc�c�v�Ƃ������������������G��, ���̔��ǃ��J�j�Y�����痈����̂ł��傤.

* Shift JIS �ȓ��{��̍������� .html �t�@�C��
* Shift JIS �ȓ��{��̒��߂����܂����v���O�����\�[�X�t�@�C��
* Mail header ���݂Ŏ�M���O�� Shift JIS �ۑ������t�@�C�� ( DKIM-Signature �������1,024�z���m�� )

�Ȃǂ�, �������������������܂�ł��傤��.

���̕��������̉����Ƃ��ăl�b�g��Łu��āv����Ă�����̂�, ���́u���z�v��

| �� | ���z |
| --- | --- |
| �J���ۂɁu�������o�v����uANSI�v�Ɏw�肵�Ȃ��� | �����J�����тɁu�������o�v�ɖ߂�̂ł���ǂ� |
| Windows10&reg; �� PC ���� Notepad.exe ���ڐA���� | �t�@�C�����j���[����J���Ɠ������ۂɑ������� |
| ���W�X�g�������ɂ傲�ɂ傷�� | �����Ȃ������c�c |
| �����Ƃ����Ƃ����G�f�B�^���g�����Ƃɂ��� | ����Ȃɑ傰���Ȃ��̂��g���قǂ̘b�ł��c�c |

�Ƃ������Ƃ���ł��傤��. ( �������܂Ōl�̊��z�ł��B)

��, �����������Ă���l�ԂƂ���
�u�������}�V�Ȏ������o�̃A���S���Y���v������������v�����Ă��܂����̂�,
�E�G�̐l�Ԃ̑̎��Łu���K���v���Ǝv���č���Ă��܂����킯�ł�.

## �@�\

* ����͂����т����肷�邮�炢�ʔ����̂Ȃ�, �����́u�������v�ł�.
* �����ނː̂Ȃ���́u�������v���x�̋@�\�����ڂ���Ă��܂�.
* ������, ����֘A�̋@�\�͓��ڂ��Ă���܂���. ~~�Ȃ񂩂߂�ǂ�������������~~ SDGs �I�ϓ_����̔��f�ł�.
* Windows11&reg; �� Notepad �ƈقȂ�, �������͂ɂ͔�Ή��ł�. ������, ���̕��y���d�オ���Ă���܂�.
* �l�b�g�ɂȂ��łǂ����ɒʐM����悤�ȋ@�\�͓����Ă���܂���.
* �Ƃ�����, �A�������@�\�͈�ؓ����Ă���܂���. �E�C���X�`�F�b�N�Ɉ������������Ƃ�����댟�o�ł�.
* �댟�o�̏ꍇ, rebuild ����Ύ��܂�悤�ł�. Hash �l���Ȃ񂩂��ς���ăp�^�[������O��邩�炩��?
* ���x rebuild ���Ă��댟�o�����܂�Ȃ��ꍇ, ����͂��Ȃ����ǉ������R�[�h�̂����ł�.

## ��

* Visual Studio 2022&reg; �ł� Windows11 ���� build �Ȃ̂�, 64bit OS �p�ł�.
* ���� build �� Windows10 �ł����Ȃ����삵�܂���, 64bit �łɌ���܂�.
* Windows7? ���������Ă����ł���? ��C�̌Ǔ��� Windows7 �𐒂߂Ă��� 7���̐M�҂������ł���?
* Windows8? ������? ����������?

## �^�p

�����ł̓C���X�g�[���[�Ȃǂ�񋟂��Ă���܂���. �P�i�� .exe ������ build ���邽�߂̃Z�b�g�ł�.
���� .exe �͂��g���� PC �̂ǂ��̃t�H���_�ɒu���Ă����܂��܂���.
( ����@�Ƃ��Ă� C:\Program Files\ �̉��ɒu���ׂ��Ȃ�ł��傤����. )

�u.txt [�t�@�C���Ƃ̊֘A�t��](https://www.google.com/search?q=�t�@�C���Ƃ̊֘A�t��)�v�Ƃ�
�u[�V���[�g�J�b�g�̍쐬](https://www.google.com/search?q=�V���[�g�J�b�g�̍쐬)�v�͂����p�Ҏ������Ă��������O��ł�.
���̋C�ɂȂ��
�u[�V���[�g�J�b�g���X�^�[�g���j���[�ɓo�^](https://www.google.com/search?q=�V���[�g�J�b�g���X�^�[�g���j���[�ɓo�^)�v
���邱�Ƃ��\�Ȃ͂��ł�.

�^�p�����, ���ׂĂ��Ȃ��������Ƃɂ���ɂ�, .exe ���폜���Ă�������.

�ݒ��ς���ƃ��W�X�g���ɋL�^���c��܂�.
������Ȃ��������Ƃɂ���ɂ� [regedit](https://www.google.com/search?q=regedit) ��
```
\HKEY_CURRENT_USER\Software\In-house Tool
```
���ۂ��ƍ폜���Ă�������. �ق��Ƃ��Ă�����ɉ���������肠��܂���.

����, ��L�̐����Łu������Ɖ������Ă邩����Ȃ��v�ȕ���, ���W�X�g���̍폜�͂�߂Ă��������������ł��傤.
���X�N�̊��ɂ͓���Ƃ��낪�Ȃ��̂�.

��������, ���X�N�ƌ�����, ����܂�̖Ɛӎ�����Y���Ă����܂��傤��.

> �����́A�{�i�̉^�p�ɂ����Đ��������̂ɂ��āA��؂̐ӔC�𕉂�Ȃ����̂Ƃ��܂��B


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

�ȂɌ����Ă�? ���[��, �܂�
�u�^�_�Ŏg���Ă�����v���Ă��Ƃ���, �����.<br>
�ł��u�g���ĂȂ񂩂����Ă��ӂ߂Ȃ��łˁv�Ƃ������Ă��.
�܁[, �^�_�Ȃ񂾂�, ����Ȃ���.

## �ӎ�

* �l�b�g��ŏ������J���Ă��������Ă���݂Ȃ��܂̂��A��, ���̃\�t�g�͏o���܂���. ���Ӑ\���グ�܂�.
* ���̃\�[�X�� Visual Studio&reg; �ō쐬���ꂽ���̂Ȃ̂�, ���̔Ō��� Microsoft&reg; ����ɂ����ӂ��Ƃ��܂�.

<div style="text-align: right;">
In-house Tool / �ƒ� �O
</div>