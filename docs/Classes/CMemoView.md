# `CMemoView`

Implemented in [`MemoView.h`](../../MemoPad/MemoPad/MemoView.h) / [`MemoView.cpp`](../../MemoPad/MemoPad/MemoView.cpp)

This class does load/edit/save, etc. almost everything to work as a simple editor like Notepad.

In general,
Visual Studio applies 
'[Document/View Architecture](https://learn.microsoft.com/en-us/cpp/mfc/document-view-architecture)'
for a MFC application like Notepad ( in the sense that application does load/edit/save ).
In that architecture,
[CWinApp](https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class)
construct a 'trinity' consists of:
* [Main Frame class](https://learn.microsoft.com/en-us/cpp/mfc/reference/cframewnd-class)
* [Document class](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdocument-class)
* [View class](https://learn.microsoft.com/en-us/cpp/mfc/reference/cview-class)

But this application did not get along with that architecture.

* Omitted the [Document class](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdocument-class)
* Use [`CEdit`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) derived class instead of the [View class](https://learn.microsoft.com/en-us/cpp/mfc/reference/cview-class)

This simple outline is enough to implement this application.

This class is a  [`CEdit`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) derived class
embedded in the client area of [Main Frame](CMainFrame.md) directly,
not via a [view](https://learn.microsoft.com/en-us/cpp/mfc/reference/cview-class).

So the class name "CMemo***View***" could be misleading.
But from other classes' point of view, it is a 'view'.
Take a general view of the 'view'.
This 'view' is written as '*view*' ( in *italic* ) in the documents of other classes.
This document also uses '*view*' hereafter.




## Constructor


### `CMemoView( void )`

The constructor

Initializes the members in this class

Values stored in the registry are read here.
The registry values read here are:

| Registry			| Meaning						|
| ---				| ---							|
| `Settings/DiscardModified`	| A switch to discard modified text without warnings	|
| `Settings/ColorText`		| Text color of the client area				|
| `Settings/ColorBack`		| Background color of the client area			|
| `Settings/RepeatTime`		| Repeat interval in [ms] to 'Replace' 'All'		|

GUI methods to modify these values are not provided in this application.
They are 'hidden' settings.
Use `Regedit` if you insist on modifying them.




## Interface Functions



### `DWORD GetStatus( void )`

Returns a bit field representing the current statuses of the editor

Exported method 1 of 1

The bit field consists of the following bits:

| Name	| Value		| Meaning							|
| ---	| --:		| ---								|
| `STAT_NOFILE`		| 1	| No file is opened yet.				|
| `STAT_EMPTY`		| 2	| The text is empty.					|
| `STAT_MODIFIED`	| 4	| The text is modified, not saved.			|
| `STAT_UNDOABLE`	| 8	| Undo is possible. ( 1 or more modification is done )	|
| `STAT_REDOABLE`	| 16	| Redo is possible. ( 1 or more Undo is done )		|
| `STAT_SELECTED`	| 32	| Some text is selected. ( Cut is possible )		|
| `STAT_PASTEABLE`	| 64	| Paste is possible. ( Clipboard contains text )	|
| `STAT_FOUND`		| 128	| Find was hit. ( Find Next / Find Prev is possible )	|

This function is called from [`CMainFrame`](CMainFrame.md) to set status of menu items.




## Overridden Functions



### `BOOL PreTranslateMessage( MSG* pMsg )`

Does specified procedures before window messages are dispatched

Overridden here for the following purpose:

1. Catch [`WM_CHAR`](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-char) messages
to replace the current character in Overwrite mode.
1. Catch [`WM_KEYDOWN`](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keydown) messages
to toggle Insert/Overwrite mode when the 'Insert' key was hit.
1. Catch [`WM_KEYUP`](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-keyup) and
[`WM_LBUTTONUP`](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttonup) messages
to renew the text in the status bar.
1. Catch [`WM_LBUTTONDBLCLK`](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-lbuttondblclk) messages
to select a word like Notepad on Windows11 ( not like Windows10 ).



## Message Handlers



### `int OnCreate( LPCREATESTRUCT lpCreateStruct )`

Handles [`WM_CREATE`](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-create)
message to insert the following actions:

1. Create [the find dialog](CFindDlg.md) as a modeless dialog.
1. Create [the font dialog](CFontDlg.md) as a modeless dialog.
1. Setup the edit control as margins, text limit, cursor and acceptance of Drag & Drop.
1. Accept a argument ( when launched with a double-click on the text file ).
1. Set indicators in the status bar.


### `void OnDestroy( void )`

Handles [`WM_DESTROY`](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-destroy)
message to insert the following actions:

1. Destroy [the find dialog](CFindDlg.md).
1. Destroy [the font dialog](CFontDlg.md).


### `void OnDropFiles( HDROP hDropInfo )`

Handles [`WM_DROPFILES`](https://learn.microsoft.com/en-us/windows/win32/shell/wm-dropfiles)
message to accept 1 drop file

When 2 or more files are dropped, just ignores.


### `BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )`

Handles [`WM_MOUSEWHEEL`](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-mousewheel)
message to zoom in/out

When the `Ctrl` key was not pressed, does as usual.
( Scrolls vertically the edit control if necessary. )


### `BOOL OnRButtonDown( UINT nFlags, CPoint point )`

Handles [`WM_RBUTTONDOWN`](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-rbuttondown)
message to popup a context menu on the editor

The last post of `WM_NULL` is a trick to solve an inconvenience of
[`TrackPopupMenu`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-trackpopupmenu#remarks)
well known by the users of Visual Studio which conceded by Microsoft at last.


### `void OnTimer( UINT_PTR nIDEvent )`

Handles [`WM_TIMER`](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-timer)
message to catch the expired timer

Following timer is used in this class:

| Timer			| Actions when expired								|
| ---			| ---										|
| `TID_REPLACEALL`	| Call [`OnFind`](#lresult-onfind-wparam-lparam-) to replace the next pattern.	|
| `TID_UNDOALL`		| Call [`Undo`](#void-undo-void-) to undo the previous modification.		|
| `TID_REDOALL`		| Call [`Redo`](#void-redo-void-) to redo the previous modification.		|
| `TID_SET_FONT`	| Set the font selected by [the font dialog](CFontDlg.md).				|
| `TID_INDICATE`	| Set the current line and column in the status bar. Set a block cursor in Overwrite mode.|


### `HBRUSH CtlColor( CDC* pDC, UINT nCtlColor )`

Handles [`WM_CTLCOLOR`](https://learn.microsoft.com/en-us/windows/win32/devnotes/wm-ctlcolor-)
message reflection to reflect the customized colors on the edit control

If the colors are not customized ( by Regedit ),
this handler does nothing and the colors are '*darkened*'.


### `void OnFileOpen( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_FILE_OPEN` in `WPARAM`
to take action for `File` menu's `Open...`

The action is as follows:

1. Call [`ConfirmDiscard()`](#bool-confirmdiscard-void-) to confirm to discard unsaved content.
	- If the answer is `Cancel` quit this action.
1. Show [the file dialog](https://learn.microsoft.com/en-us/cpp/mfc/reference/cfiledialog-class)
to select a file to open.
	- If the answer is `Cancel` quit this action.
1. Call [`LoadFile()`](#bool-loadfile-cstring-strfile-) to open the specified file.


### `void OnFileSave( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_FILE_SAVE` in `WPARAM`
to take action for `File` menu's `Save`

The action is as follows:

1. If the text is newly created ( not loaded from an existing file ),
call [`OnFileSaveAs()`](#void-onfilesaveas-void-) to save as a new file.
1. If the text is loaded from an existing file,
get the 'lowest encode' ( the minimum level encoding required by the text )
and save the file with the encoding.
1. Anyway, store the saved text as 'original text' ( not modified ).
1. Clear 'modified flag'.
1. Renew window title with the file name without 'modified mark'.


### `void OnFileSaveAs( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_FILE_SAVE_AS` in `WPARAM`
to take action for `File` menu's `Save as...`

The action is to show
[the file dialog](https://learn.microsoft.com/en-us/cpp/mfc/reference/cfiledialog-class)
to save a file.
And save into the specified file if the `OK` button was hit in the dialog.

* The dialog is set 'lowest encode' before open a file.
* The encoding selected by user is retrieved from the dialog.
* 'modified flag' and 'modified mark' are both removed.


### `void OnFileClose( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_FILE_CLOSE` in `WPARAM`
to take action for `File` menu's `Close`<br>
( This message is posted to this '*view*' when the application was closed.
 See [CMainFrame::PreTranslateMessage()](CMainFrame.md#bool-pretranslatemessage-msg-pmsg-). )

The action is as follows:

1. Call [`ConfirmDiscard()`](#bool-confirmdiscard-void-) to confirm to discard unsaved content.
	- If the answer is `Cancel` quit this action.
1. Post `WM_CLOSE` to the [main window](CMainFrame.md) to close the application process.

Closing the file means closing the application in case of MemoPad
like the case of the last file was closed in Notepad
( MemoPad opens only one file, so the opened file is the last file always ).


### `void OnFilePrint( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_FILE_PRINT` in `WPARAM`
to take action for `File` menu's `Print...`

The action is to show
[the print dialog](CPrintDlg.md)
to select a printer, paper size, orientation and margins.
And print the text if the `OK` button was hit in the dialog.


### `void OnFileProperties( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_FILE_UPDATE` in `WPARAM`
to take action for `File` menu's `Properties`

( The ID `ID_FILE_UPDATE` is not concerned with 'Properties',
just applied as a unique identifier from the stock of predefined IDs. )

The action is to show [the property dialog](CPropertyDlg.md) with the properties of the file.

'bytes on file', one of the properties, is calculated by
[`GetSizeOnFile`](#dword-getsizeonfile-cstring-strfile-bool-bmodified-)
even if the content of '*view*' is modified.


### `void OnEditUndo( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_UNDO` in `WPARAM`
to take action for `Edit` menu's `Undo`

The action is to call
[`CMemoView::Undo`](#void-undo-void-) to 'undo' the last modification.


### `void OnEditRedo( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_REDO` in `WPARAM`
to take action for accelerator `Ctrl` + `Y`<br>
( This command is not in the menu. Why? Notepad is doing so. )

The action is to call
[`CMemoView::Redo`](#void-redo-void-) to 'redo' the last 'undo'.


### `void OnEditCut( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_CUT` in `WPARAM`
to take action for `Edit` menu's `Cut`

The action is to call
[`CEdit::Cut`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class#cut)
to 'cut' the selected text.


### `void OnEditCopy( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_COPY` in `WPARAM`
to take action for `Edit` menu's `Copy`

The action is to call
[`CEdit::Copy`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class#copy)
to 'copy' the selected text into the Clipboard


### `void OnEditPaste( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_PASTE` in `WPARAM`
to take action for `Edit` menu's `Paste`

The action is to call
[`CEdit::Paste`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class#paste)
to 'paste' the text in the Clipboard into the current position.


### `void OnEditClear( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_CLEAR` in `WPARAM`
to take action for `Delete` in the context menu on this '*view*'

The action is to call
[`CEdit::Clear`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class#clear)
to 'clear' the selected text.


### `void OnEditFind( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_FIND` in `WPARAM`
to take action for `Edit` menu's `Find`

The action is to popup
[the find dialog](CFindDlg.md) in 'find' mode
on the right bottom corner with the selected text.


### `void OnEditFindNext( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_NEXT_PANE` in `WPARAM`
to take action for `Edit` menu's `Find next`

( The ID `ID_NEXT_PANE` is not concerned with 'Find next',
just applied as a unique identifier from the stock of predefined IDs. )

The action is to call
[`CMemeView::OnFind()`](#void-onfind-wparam-lparam-)
with the command `FIND_COMMAND_NEXT` to find the next pattern.


### `void OnEditFindPrev( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_PREV_PANE` in `WPARAM`
to take action for `Edit` menu's `Find next`

( The ID `ID_PREV_PANE` is not concerned with 'Find next',
just applied as a unique identifier from the stock of predefined IDs. )

The action is to reverse the direction of search and to call
[`CMemeView::OnFind()`](#void-onfind-wparam-lparam-)
with the command `FIND_COMMAND_NEXT` to find the previous pattern.


### `void OnEditReplace( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_REPLACE` in `WPARAM`
to take action for `Edit` menu's `Replace`

The action is to popup
[the find dialog](CFindDlg.md) in 'replace' mode
on the right bottom corner with the selected text.


### `void OnEditSelectAll( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_SELECT_ALL` in `WPARAM`
to take action for `Edit` menu's `Select all`

The action is to select all text in the '*view*'.


### `void OnEditInsertUnicode( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_EDIT_PASTE_SPECIAL` in `WPARAM`
to take action for accelerator `Alt` + `X`<br>
( This command is not in the menu. )

The action is to insert a Unicode character
when the selected text is 4 or 5 digit hexadecimal value.

( The ID `ID_EDIT_PASTE_SPECIAL` is not concerned with Unicode,
just applied as a unique identifier from the stock of predefined IDs. )


### `void OnViewFont( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_FORMAT_FONT` in `WPARAM`
to take action for `View` menu's `Font...`

( The ID `ID_FORMAT_FONT` is not concerned with font selection,
just applied as a unique identifier from the stock of predefined IDs. )

The action is to popup
[the font dialog](CFontDlg.md)
on the right bottom corner to select a font to display text in the '*view*'.


### `void OnViewZoomIn( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_WINDOW_CASCADE` in `WPARAM`
to take action for `View` menu's `Zoom in`

( The ID `ID_WINDOW_CASCADE` is not concerned with zoom,
just applied as a unique identifier from the stock of predefined IDs. )

The action is to increment the font size by 1 point.

But the font is not resized immediately in this thread but a timer of ( about ) 20ms is set.
When the timer is expired, the resizing is executed.
This is a time-domain 'buffer' for the case the size was changed by
[mouse wheel](#bool-onmousewheel-uint-nflags-short-zdelta-cpoint-pt-),
since the mouse wheel message can be generated in short interval against the time taken to redraw the '*view*'.


### `void OnViewZoomOut( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_WINDOW_ARRANGE` in `WPARAM`
to take action for `View` menu's `Zoom out`

( The ID `ID_WINDOW_ARRANGE` is not concerned with zoom,
just applied as a unique identifier from the stock of predefined IDs. )

The action is to decrement the font size by 1 point.

But the font is not resized immediately in this thread but a timer of ( about ) 20ms is set.
When the timer is expired, the resizing is executed.
This is a time-domain 'buffer' for the case the size was changed by
[mouse wheel](#bool-onmousewheel-uint-nflags-short-zdelta-cpoint-pt-),
since the mouse wheel message can be generated in short interval against the time taken to redraw the '*view*'.


### `void OnViewZoomRestore( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_WINDOW_NEW` in `WPARAM`
to take action for `View` menu's `Restore zoom`

( The ID `ID_WINDOW_NEW` is not concerned with zoom,
just applied as a unique identifier from the stock of predefined IDs. )

The action is to restore the original font size.


### `void OnEditUpdate( void )`

Handles [`EN_UPDATE`](https://learn.microsoft.com/en-us/windows/win32/controls/en-update)
message reflection to store the last image before the modification

This image is used to compare with the image after modification.


### `void OnEditChange( void )`

Handles [`EN_CHANGE`](https://learn.microsoft.com/en-us/windows/win32/controls/en-change)
message reflection to store the last image after the modification

Then call [`TakeDiff()`](#void-takediff-void-) to compare the images before and after the modification.

On the way to do this,
call [`RenewTitle()`](#void-renewtitle-void-) to set/clear 'modified mark'.


### `LRESULT OnFind( WPARAM wParam, LPARAM lParam )`

Handles `WM_FIND` message ( defined in [`CFindDlg`](CFindDlg.md) )
to notify the operation on [`CFindDlg`](CFindDlg.md) to this '*view*'

The following procedure is done for this message:

1. Take the parameters ( in `LPARAM` ) sent from [`CFindDlg`](CFindDlg.md).
1. If the operation was `Replace` ( one by one ), replace the current selection ( the last found text ).
1. If the operation is not 'Match case', convert to lower case both the text in '*view*' and the pattern to find.
1. Set the start point to search.
1. Search the pattern to find.
1. If the pattern was found, do replace for 'Replace All' request and set a timer to next replace,<br>
do selection for 'Find' request.
1. Show line and column in the status bar.


### `LRESULT OnFont( WPARAM wParam, LPARAM lParam )`

Handles `WM_FONT` message ( defined in [`CFontDlg`](CFontDlg.md) )
to notify the selection on [`CFontDlg`](CFontDlg.md) to this '*view*'

The following procedure is done for this message:

1. Take the `LOGFONT` selected by [`CFontDlg`](CFontDlg.md) ( in `LPARAM` ).
1. Call [`SetFont()`](#void-setfont-logfont-) to set.

Unlike [`OnViewZoomIn()`](#void-onviewzoomin-void-), setting of the font is executed immediately,
since you cannot push the button on [`CFontDlg`](CFontDlg.md) so frequently.



## Specific Functions



### `bool LoadFile( CString strFile )`

Loads the given file into the '*view*'
and returns `true` when the loading is completed.

Loading the file is done in the following steps:

1. Open the file with read mode. Return `false` if failed to open.
1. Allocate a buffer and read the file into the buffer as a binary image.
1. Call [`GetTextEncode()`](#void-gettextencode-byte-qword-) to get encoding of the file.
1. Set the image according to the encoding:
	- `ASCII` or `ShiftJIS`: Set the image into the '*view*' as single-byte characters.
	- `UTF8`: Convert the image from UTF8 to Unicode, and then set into the '*view*' as wide characters.
	- `UTF16LE`: Set the image into the '*view*' as wide characters.
	- `UTF16BE`: Convert the image from BE to LE, and then set into the '*view*' as wide characters.
	- Others: Empty the '*view*'.
1. Free the buffer.
1. Set the window title from the file name.
1. Clear 'undo buffer' and 'redo index'.
1. Return `true`.


### `bool SaveFile( CString strFile )`

Saves the content of '*view*' into the given file and
returns `true` when the saving is completed

Saving the file is done in the following steps:

1. Open the file with write ( and create ) mode. Return `false` if failed to open.
1. Save the content according to the encoding of it:
	- `ASCII` or `ShiftJIS`: Set the content of '*view*' into the buffer as single-byte characters.
	- `UTF8`: Convert the content of '*view*' from Unicode to UTF8, and then set into the buffer.
	- `UTF16LE`: Set the content of '*view*' into the buffer as wide characters.
	- `UTF16BE`: Convert the content of '*view*' from LE to BE, and then set into the buffer as wide characters.
1. If BOMs are attached, insert them on the top of the buffer.
1. Write the buffer into the file.
1. Free the buffer.
1. Return `true`.

If the saving was failed, call
[`AfxMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display#afxmessagebox)
to notify that. But that
[`AfxMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display#afxmessagebox)
is '*darkened*' by the work of
[`CMemoApp::DoMessageBox()`](CMemoApp.md#int-domessagebox-lpctstr-lpszprompt-uint-ntype-uint-nidprompt-).


### `bool ConfirmDiscard( void )`

Calls
[`AfxMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display#afxmessagebox)
to confirm whether to save the unsaved ( but modified ) content of '*view*'

The options are:

| Option		| Action					|
| ---			| ---						|
| `Yes`			| Save the content and returns `true`.		|
| `No`			| Returns `true` without saving the content.	|
| `Cancel`		| Returns `false`.				|

When `false` was returned, the caller should cancel the following procedures ( like to open a new file ).


### `DWORD GetSizeOnFile( CString strFile, bool bModified )`

Returns the file size in byte of the given file

If the file is not modified, the result is simple.
The physical size on the filesystem is returned.
But this function can return the byte count of a modified file.
In that case, the byte count is calculated as follows:

1. Get the content in '*view*'.
1. If the line-termination is 'LF', replace all 'CR/LF' to 'LF' in the content.
1. Calculate the byte count according to the encoding of the content:
	- `ASCII` or `ShiftJIS`: Convert the content into single-byte text, then get length of the string.
	- `UTF8`: Get length as a UTF8 sequence.
	- `UTF16LE`: Calculate number of characters &times; sizeof wide-byte character.
	- `UTF16BE`: Calculate number of characters &times; sizeof wide-byte character.
1. Add byte count of BOM.

This function is called from
[`OnFileProperties`](#void-onfileproperties-void-)
to show the size of current file even if the file is modified.


### `DWORD GetSizeOnDisk( CString strFile, DWORD cbFile )`

Returns count of byte occupied on the filesystem for the given file

Gets how many clusters are occupied by the file.
Then returns 'cluster size &times; cluster count'.


### `void GetTextEncode( BYTE* pbData, QWORD cbData )`

Detect text encoding of the given content image

The detection is done as follows:

1. If the content is empty, the encoding is set as `unknown`.
1. If the BOMs are set, select the encoding according to the BOMs as `UTF8`, `UTF16LE` and `UTF16BE`.
1. If the BOMs are not set, scan the each character code in the content as follows:
	- The code less than `0x7f`: skip the assumption.
	- The codes are UTF8 sequence: assume it to be `UTF8`.
	- The codes of 2bytes are in the code area of Shift JIS: assume it to be `ShiftJIS`.
1. If no assumptions were made, set the encoding as `ASCII`.
1. If the assumptions match enough times ( 16 times here ), set the encoding as the assumption.
1. If the assumptions did not match enough, set the encoding as `unknown`.
1. Then determine the line-termination as 'CR/LF' or 'LF'.
1. Indicate the encoding and the line-termination in the status bar.

The idea of this algorithm was a main motive to make this application.
The 'Auto-detect' of encoding built in the original Notepad is unsatisfying,
since the detection looks only 1,024 bytes from the top of the text file,
resulting wrong detection as 'UTF8' for Shift JIS text.


### `void GetLowestEncode( Encode& eEncode )`

Get 'lowest' text encoding of the current content in the '*view*'

The 'lowest' means 'the encoding which requires the least byte count to contain'.<br>
The order of the encoding from the byte count's point of view is as follows:

| Encoding	| Count of bytes							|
| ---		| ---									|
| `ASCII`	| Bytes &#xff1d; Count of characters					|
| `ShiftJIS`	| Bytes &#xff1d; Count of characters &#xff0b; Count of Shift JIS code	|
| `UTF8`	| `ASCII` &#x2266; Bytes &#x2266; `UTF16`				|
| `UTF16`	| Bytes &#x2252; Count of characters &times; 2				|

This function selects the encoding as follows:

1. Every code is less than `0x7f`, the encoding is `ASCII`.
1. If the content was converted to Shit JIS, the encoding is `ShiftJIS`.
1. If the conversion was failed, the encoding is `UTF8`.

For example, a text includes emoji characters cannot be encoded in Shift JIS.
In such a case, this function gives `UTF8` and this '*view*' limits the selection of encoding in
[the file dialog](https://learn.microsoft.com/en-us/cpp/mfc/reference/cfiledialog-class)
not to select `ASCII` nor `ShiftJIS`.


### `void SetDefaultEncode( CFileDialog& dlg, Encode eEncode )`

Set default encoding to [the file dialog](https://learn.microsoft.com/en-us/cpp/mfc/reference/cfiledialog-class)

This function limits the options of encoding and set default.


### `void GetSpecifiedEncode( CFileDialog& dlg )`

Get selected encoding from [the file dialog](https://learn.microsoft.com/en-us/cpp/mfc/reference/cfiledialog-class)

Get the encoding which user chosen.

This '*view*' made a proposal to minimize the file size.
But it's a user's choice to save into a larger file...


### `void SetTitle( CString strFile )`

Set text on the title bar

The text on the title bar consists of
* '&#x2731;' as a mark to notify that the content was modified but not saved yet
* File name of the current text file
* Application name as 'MemoPad'

This function sets the text except the mark for newly opened / saved file.


### `void RenewTitle( void )`

Renew mark on the title bar.

This function checks if the content is modified from the original image,
add or remove '&#x2731;' as a mark to notify that the content was modified but not saved yet.

This function is called when the file is saved ( to remove the mark ),
when the modification was notified from
[`CEdit`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class)
( to add or remove the mark ) and
when Undo / Redo was done ( to add or remove the mark ).


### `void TakeDiff( void )`

Take a difference between before and after a modification

This '*view*' support multi-stage Undo ( and Redo ).
But the base class of this '*view*'
[`CEdit`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class)
supports only one-stage Undo ( and no Redo ).
So the multi-stage Undo/Redo must be implemented in this '*view*'.

To implement multi-stage Undo/Redo,
the '*view*' must record following information at each modification:

* What is done ( inserted / deleted / replaced )
* What is changed ( the text inserted / deleted / replaced )
* Where is changed ( the index in the [`CEdit`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class) )

These information is packed in the class `CUndo`, and the array of `CUndo` is a 'Undo Buffer' in this '*view*'.

`TakeDiff` is called from [`OnEditChange`](#void-oneditchange-void-)
to compare the 'new image after a modification' with the 'old image before the modification'
left by [`OnEditUpdate`](#void-oneditupdate-void-). The comparison is done from 2 points:

1. Compare 'old' and 'new' from the top of the content toward the bottom.
1. Compare 'old' and 'new' from the bottom of the content toward the top.

This 2 points of view catches the difference.

Consider a case that `"The Notepad.\r\n"` from column 1 ( index 0 )
was replaced with `"The MemoPad.\r\n"`.


1st, `"Notep"` was deleted.
( [`CEdit`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class)
does not notify as 'Replaced' but each step as 'Deleted' and 'Inserted'. )

`Old`: ( before the modification )
|  0|  1|  2|  3|  4|  5|  6|  7|  8|  9| 10| 11| 12| 13|
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| T | h | e |   | N | o | t | e | p | a | d | . | \r| \n|

`New`: ( after the modification )
|  0|  1|  2|  3|  4|  5|  6|  7|  8|
|---|---|---|---|---|---|---|---|---|
| T | h | e |   | a | d | . | \r| \n|

* Comparison from the top detects `New[4] != Old[4]`.
* Comparison from the bottom detects `New[3] != Old[8]`.

Take the difference as:
* Index from the bottom on `New` ( = `3` ) &#xff1c; index on `Old` ( = `8` ): The modification was 'Delete'
* The length of difference is 5 ( = `8` - `3` )
* The text deleted is 5 characters from `Old[4]` = `"Notep"`

The 1st difference is packed in a `CUndo` as:

|`m_iChar`	| 4		|
|:---		|:---		|
|`m_strText`	| `"DNotep"`	|

The `D` at the top of `m_strText` means 'Delete'.

This `CUndo` is added to the 'Undo Buffer'.

2nd, `"MemoP"` was added.

`Old`: ( before the modification )
|  0|  1|  2|  3|  4|  5|  6|  7|  8|
|---|---|---|---|---|---|---|---|---|
| T | h | e |   | a | d | . | \r| \n|

`New`: ( after the modification )
|  0|  1|  2|  3|  4|  5|  6|  7|  8|  9| 10| 11| 12| 13|
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| T | h | e |   | M | e | m | o | P | a | d | . | \r| \n|

* Comparison from the top detects `New[4] != Old[4]`.
* Comparison from the bottom detects `New[8] != Old[3]`.

Take the difference as:
* Index from the bottom on `New` ( = `8` ) &#xff1e; index on `Old` ( = `3` ): The modification was 'Insert'
* The length of difference is 5 ( = `8` - `3` )
* The text inserted is 5 characters from `New[4]` = `"MemoP"`

The 2nd difference is packed in a `CUndo` as:

|`m_iChar`	| 4		|
|:---		|:---		|
|`m_strText`	| `"IMemoP"`	|

The `I` at the top of `m_strText` means 'Insert'.

Before adding this `CUndo` to the 'Undo Buffer',
the following condition is checked:

* Is it an 'Insert' after a 'Delete' at the same index?

If the answer is 'yes', that 'Delete' & 'Insert' is merged into one 'Replace' as:

|`m_iChar`	| 4			|
|:---		|:---			|
|`m_strText`	| `"RNotep\bMemoP"`	|

The `R` at the top of `m_strText` means 'Replace'.
The `\b` is a separator between the old text and the new text.

The last 'Insert' in 'Undo Buffer' is overwritten with this 'Replace'.


### `BOOL CanUndo( void )`

Returns whether Undo is possible

If one or more modification was done, Undo is possible in this '*view*'.

This is one of the flags returned by [`GetStatus()`](#dword-getstatus-void-).


### `BOOL CanRedo( void )`

Returns whether Redo is possible

If one or more Undo was done, Redo is possible in this '*view*'.

This is one of the flags returned by [`GetStatus()`](#dword-getstatus-void-).


### `void Undo( void )`

Undo one modification

Modification history is stored in the 'Undo Buffer' of the '*view*'.
One Undo is to go back a modification.
The index `m_iUndo` represents where to go back.
If no Undo is done, `m_iUndo` points the last modification.
One Undo is done, `m_iUndo` decrements by one which means 'went back one stage'.
If the all modifications were Undo'ed, `m_iUndo` reaches `0` which means 'no more Undo is possible'.

So one Undo is to do:
* Decrement `m_iUndo` by one
* Do the reverse of 'Undo Buffer' s `m_iUndo`th modification

The reverse is to do:
* Delete the inserted text for 'Insert'
* Insert the deleted text for 'Delete'
* Exchange the new text with the old text for 'Replace'

After a Undo, calls [`RenewTitle()`](#void-renewtitle-void-) to maintenance the mark beside title.


### `void Redo( void )`

Redo one undo

As explained in [`Undo()`](#void-undo-void-), one Undo decrements `m_iUndo` by one.

Consider a case just after 3 modifications are done.
At that time, the 'Undo Buffer' is stacked like this:

|index	|content		|
|---:	|---			|
|`[2]`	| 3rd modification	|
|`[1]`	| 2nd modification	|
|`[0]`	| 1st modification	|

Now, `m_iUndo` is set as 3 equals to the size of the history.

Then, 2 Undos are done. `m_iUndo` is 1, pointing at the 2nd modification.

|index	|content		|
|---:	|---			|
|`[2]`	| 3rd modification	|
|>`[1]`	| *2nd modification*	|
|`[0]`	| 1st modification	|

Then, 1 Redo is done. `m_iUndo` is 2, pointing at the 3rd modification.

|index	|content		|
|---:	|---			|
|>`[2]`	| *3rd modification*	|
|`[1]`	| 2nd modification	|
|`[0]`	| 1st modification	|

So one Redo is to do:
* Do the same of 'Undo Buffer' s `m_iUndo`th modification
* Increment `m_iUndo` by one

If the all Undos were Redo'ed, `m_iUndo` reaches size of the 'Undo Buffer' which means 'no more Redo is possible'.


### `void SelectWord( void )`

Selects a word

Called from [`PreTranslateMessage()`](#bool-pretranslatemessage-msg-pmsg-)
to select a word according to the current cursor position.

This selection is done like Notepad on Windows11 ( not like Windows10 ),
except 'end-of-line' is not selected.


### `UINT TypeOfChar( TCHAR ch )`

Returns the type of given character

This function return the type of given character as follow.

| Return value	| Character type		|
| ---:		| :---				|
| `0`		| Control characters and spaces	|
| `1`		| Signs				|
| `2`		| Others			|

The return value is used to separate a word in `SelectWord()`.


### `void SetFont( LOGFONT* plf )`

Sets the font of the '*view*' with given `LOGFONT`

Called from [`OnTimer()`](#void-ontimer-uint_ptr-nidevent-)
to set the size of the font
and from [`OnFont()`](#lresult-onfont-wparam-wparam-lparam-lparam-)
to set the font selected by [`CFontDlg`](CFontDlg.md).


### `CString GetSelected( void )`

Returns a selected text by [`CEdit`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class)


### `bool IsPasteable( void )`

Returns whether a text is pasteable from Clipboard


### `CString CommaDigitsOf( int nValue )`

Returns a string decoded in comma separated decimal for the given value.


### `bool Print( CPrintParam& param )`

Prints the current content in '*view*' with given parameters

Called from [`OnFilePrint()`](#void-onfileprint-void-)
with the parameters given from [the print dialog](CPrintDlg.md),
and do the followings to print out the content:

1. Call [`OpenPrinter()`](https://learn.microsoft.com/en-us/windows/win32/printdocs/openprinter)
to open the printer specified in the parameter.
1. Call [`DocumentProperties()`](https://learn.microsoft.com/en-us/windows/win32/printdocs/documentproperties)
2 times to
	- get size of the property block
	- get the property of the printer
1. Allocate a block of a
[device mode](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/the-devmodew-structure).
1. Set the paper size specified in the parameter into the
[device mode](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/the-devmodew-structure).
1. Set the paper orientation specified in the parameter into the
[device mode](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/the-devmodew-structure).
1. [Create a device context](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class#createdc)
for
[device mode](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/the-devmodew-structure)
in the name of driver as `"WINSPOOL"`.
1. [Start the document](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class#startdoc)
in the device context with a unique job name made by [`MakeJobName()`](#cstring-makejobname-void-).
1. Call [`PrintContent()`](#void-printcontent-cdc-pdc-cprintparam-param-) to print the content in the '*view*'.
1. [Delete the device context](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class#deletedc).
1. Release the
[device mode](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/the-devmodew-structure).
1. Call [`ClosePrinter()`](https://learn.microsoft.com/en-us/windows/win32/printdocs/closeprinter)
to close the printer.

Above is a generic procedure to print out something to a printer.
With
'[Document/View Architecture](https://learn.microsoft.com/en-us/cpp/mfc/document-view-architecture)',
this sort of procedure is implemented and executed in
[`CView`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cview-class).
But this '*view*' is not a
[`CView`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cview-class)
derived class so the procedure is implemented here like this.

This source may be a hint to add print function to 
Non-[Document/View Architecture](https://learn.microsoft.com/en-us/cpp/mfc/document-view-architecture)
applications like dialog-based ones.


### `void PrintContent( CDC* pDC, CPrintParam& param )`

Prints the current content in '*view*' with given parameters and device context

Called from [`Print()`](#bool-print-cprintparam-param-)
with the parameters given from [the print dialog](CPrintDlg.md), the device context of the printer
and do the followings to print out the content:

1. [Set map mode](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-setmapmode)
as `MM_LOENGLISH`.
1. Select a font to print out as same as '*view*'.
1. Set the text color as black.
1. Calculate the followings in this device context:
	- Size of the header
	- Size of the footer
	- Size of the page without margins
	- Height of 1 line
1. [Set the viewport origin](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class#setviewportorg).

Then do the followings print out a page:

1. Call [`PrintMargin()`](#) to print out the header.
1. Retrieve one line to print.
1. Call [`PrintLine()`](#void-printline-cdc-pdc-crect-rectdraw-cstring-strline-int-cyline-int-ntab-)
to calculate the size of the line.
1. If the size is too large to print in the left space of the page,
break the line to fit in the space.
1. Call [`PrintLine()`](#void-printline-cdc-pdc-crect-rectdraw-cstring-strline-int-cyline-int-ntab-)
to print out the line.
1. Skip to the next line.
1. If the left space can contain the next line, repeat from 2.
1. Call [`PrintMargin()`](#) to print out the footer.

When all pages are done, [end the document](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class#enddoc).

This procedure is complicated with these premises:

* All printings must be kept in the page space.
* But the device context of printer does not support 'Word break'.

So, before print out text, the space required for the print out must be calculated.
If the left space is not enough, the text must be truncated to meet the edge of the space
or must be skipped to the next page.
The following functions are for these calculation, truncation and print out.


### `void PrintLine( CDC* pDC, CRect& rectDraw, CString& strLine, int cyLine, int nTab )`

Calculates a space required for the text and break the text to fit in the space ( when `cyLine` &#xff1d; `0` ) or
<br>draws the broken text to the device context of printer ( when `cyLine` &#x2260; `0` )

This function is called 2 times. The job is done in 2 stages.

#### 1st stage: calculate and break

If this function was called with the line height &#xff1d; `0` ( `cyLine` &#xff1d; `0` ),<br>
calculates the size required for the given text.
If the text was too long to set in the given rectangle,
the text was 'broken' ( 'CR/LF' was inserted ) not to go over the right edge of the rectangle.
There are 2 types of 'break'.

| Type		| How it breaks						|
| ---		| ---							|
| Word break	| 'Break' at the space or tab nearest the right edge	|
| Hard break	| 'Break' at the physical right edge			|

'Word break' is preferred, but no space characters are in the text, `Hard break' is chosen.

The 'broken' lines are increased in the count of line. So the required height is increased.
A too long text may go over the bottom of the page.
Such a case is cared in [`PrintContent()`](#void-printcontent-cdc-pdc-cprintparam-param-)
which is aware of the distance to the bottom.

The rectangle `rectDraw` is enlarged to contain the 'broken' lines,
the text `strLine` is 'broken' to fit the rectangle.

#### 2nd stage: print out

If this function was called with the line height &#x2260; `0` ( `cyLine` &#x2260; `0` ),<br>
draws each line into the given device context.

The lines are already 'broken' to fit the width of the page.
Each line is drawn with the given line height `cyLine`.

In both stage, tabs are calculated originally to make appropriate space.
This calculation is based on the graphical width of characters, not on the character count.
So the space is steady even if a proportional font is applied.


### `void BreakLine( CDC* pDC, CRect& rectLine, CString& strLine, int& cx, UINT uFormat )`

'Break's the given text with maximum width within the given rectangle

This function calculates the size of text on one-by-one basis, until the maximum text length is found.
The maximum text is broken with 'CR/LF'. Repeat this until all text is 'broken'.

The text given to this function is a token separated by tabs.
Tabs are calculated in 
[`PrintLine()`](#void-printline-cdc-pdc-crect-rectdraw-cstring-strline-int-cyline-int-ntab-).


### `void PrintMargin( CDC* pDC, CRect rectMargin, CString strMargin, int nPage )`

Prints a header/footer text in the margin area of the page

The text format is compatible with
[the commands in Notepad](https://support.microsoft.com/en-us/windows/changing-header-and-footer-commands-in-notepad-c1b0e27b-497d-c478-c4c1-0da491cac148).

Plus, the command `"&b"` is added in this '*view*'.
This `"&b"` is a original command to draw a border line between the margin and the text body.

### `bool IsPageToPrint( UINT uPage, CUIntArray& uaPages )`

Returns whether the given page number is to print out or not

[The print dialog](CPrintDlg.md) can specify the range of pages to print out.
Given argument `uaPages` is the pages specified in [the print dialog](CPrintDlg.md).
Referring that, this function returns `true` for the pages to be printed, `false` to the pages to be skipped.


### `CString MakeJobName( void )`

Returns a print job name

This function makes a print job name according to:

* The file name to print
* The user account name who entered the print job
* The host name which entered the print job

like:

`Explanations.txt - Fumio.KISHIDA@KishidaNotePC`

to distinguish who enterd the print job,
just in case the job was stalled in the print server.
( In the case of above example, the administrator of the server would inform KISHIDA like
"Hi, Fumio! Your explanations are stalled!" )
