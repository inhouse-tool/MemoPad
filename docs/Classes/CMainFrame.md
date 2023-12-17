# `CMainFrame`

Implemented in [`MainFrm.h`](../../MemoPad/MemoPad/MainFrm.h) / [`MainFrm.cpp`](../../MemoPad/MemoPad/MainFrm.cpp)

This class draws a 'Main Frame' of this application.

This class owns an instance of [`CDark`](CDark.md) to '*darken*' the main frame itself, a '*view*' in it,
and the dialogs belong to the '*view*'.
To reduce memory consumption, this instance is 'shared' to other windows and dialogs in the process.
This class provides some macros to use the [`CDark`](CDark.md) instance from those windows.

By the way, the word '*view*' is a technical term came from the
'[Document/View Architecture](https://learn.microsoft.com/en-us/cpp/mfc/document-view-architecture)'
invented by Microsoft.
The '*view*' in this page means the similar object ( especially from the Main Frame's point of view )
( in spite of that
'[Document/View Architecture](https://learn.microsoft.com/en-us/cpp/mfc/document-view-architecture)'
is not adopted in this application actually ).


## Macros

### `SupportDarkMode( hWnd )`

'*Darken*'s the given window

Give a window handle as an argument of this macro then the window is '*darkened*'.

### `MessageDlg( lpszPrompt, nType )`

Popups a '*darkened*'
[`AfxMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display#afxmessagebox)

Give a prompt text as 1st argument and type of style as 2nd argument like
[`AfxMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display#afxmessagebox).
Then a '*darkened*' message box popups.
Its return value is compatible with
[`AfxMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display#afxmessagebox).

### `SetIndicator( iPane, lpszText )`

Sets the text in status bar

Give the pane number ( 0 based ) as 1st argument and text to show as 2nd argument
then the given text is shown in specified pane of the status bar.


## Constructor & Destructor


### `CMainFrame( void )`

The constructor

This function initializes the members in this class.
Values stored in the registry are read here.
The registry values read here are:

| Registry		| Meaning						|
| ---			| ---							|
| `Settings/WordWrap`	| Show the text in '*view*' with 'Word wrap' or not	|
| `Settings/MenuBar`	| Show the menu bar or not				|
| `Settings/StatusBar`	| Show the status bar or not				|



### `~CMainFrame( void )`

The destructor

Registry values are stored here if they are modified.

If nothing is changed, nothing is stored.
They say that writing so frequently this kind of non-volatile data is not good
for your ( NAND flash-based ) SSD.



## Overridden Functions



### `void GetMessageString( UINT nID, CString& rMessage ) const`

Returns a string for resource ID

This method is to override the message strings correspond the resource IDs.

Overridden here to replace `AFX_IDS_IDLEMESSAGE` which is shown as `"Ready"` by default
with 'Line and Column' indicator string `"Lin %d, Col %d"`.


### `BOOL PreCreateWindow( CREATESTRUCT& cs )`

Does specified procedures before creating the window

Overridden here to change the name of window class as `"MemoPad"`.

This kind of trick is often done by those who want to narrow range of
[`FindWindow`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-findwindoww).


### `BOOL PreTranslateMessage( MSG* pMsg )`

Does specified procedures before window messages are dispatched

Overridden here for the following purpose:

1. Catch window messages which close the main frame directly.
Replace them with the action to close the '*view*'.
This is done to close the file gracefully ( not to leave the unsaved file ).
1. Catch window messages for keyboard input.
This is done to apply the keyboard accelerator.


### `BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )`

Does specified procedures before commands are dispatched

Overridden here to handle the command messages by the '*view*' before the main frame,
since some commands can not be done without this precedence.



## Message Handlers

### `int OnCreate( LPCREATESTRUCT lpCreateStruct )`

Handles [`WM_CREATE`](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-create)
message to insert the following procedures:

1. Set the icons and the keyboard accelerators.
1. Initialize the window title.
1. Create a client '[*view*](CMemoView.md)'.
1. Insert menus on the menubar into the system menu.
1. Set timer to place the window of main frame.

No.4. is a remedy for the users who selected to hide `Menu bar` and regretting it.
The menu bar can be restored by the system menu. Right-click the title bar and select `View` menu.

No.5 is done to place the window as fast as possible ( with the timer value '0' ).
The reason to insert a timer is such a procedure to place a window can not be done in this thread of `OnCreate()`.

### `void OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu )`

Handles [`WM_INITMENUPOPUP`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-initmenupopup)
message to set status ( enabled or disabled / checked or unchecked ) of each command in the system menu

Most of the statuses come from the '[*view*](CMemoView.md)'.
See [`CMemoView::GetStatus()`](CMemoView.md#dword-getstatus-void-) for details.


### `void OnSetFocus( CWnd* pOldWnd )`

Handles [`WM_SETFOCUS`](https://learn.microsoft.com/en-us/windows/win32/inputdev/wm-setfocus)
message to hand over keyboard focus to the '*view*' window

This handler is always generated by Visual Studio as it is.
There is no need to modify it.


### `void OnSettingChange( UINT uFlags, LPCTSTR lpszSection )`

Handles [`WM_SETTINGCHANGE`](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-settingchange)
message to detect a change of `ImmersiveColorSet`

If the 'section' ( given as `LPARAM` ) was `"ImmersiveColorSet"`,
it means that mode was changed between 'Dark Mode' and 'Light Mode'.
This handler reposts
[`WM_SYSCOLORCHANGE`](https://learn.microsoft.com/en-us/windows/win32/gdi/wm-syscolorchange)
to the main frame itself.


### `void OnSysColorChange( void )`

Handles [`WM_SYSCOLORCHANGE`](https://learn.microsoft.com/en-us/windows/win32/gdi/wm-syscolorchange)
message to destroy and create the client '*view*'

The reason why such actions are done here is
scroll bars in the client '*view*' can not respond the change of the mode.
Once the scroll bar is created in 'Dark Mode', it keeps 'Dark' even if the mode was changed later.
So the client '*view*' with old mode must be destroyed before a new client '*view*' with new mode is created.


### `void OnSysCommand( UINT nID, LPARAM lParam )`

Handles [`WM_SYSCOMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-syscommand)
message to accept menu items inserted in the system menu

This handler repost `WM_COMMAND` to the main frame itself with same `WPARAM`
which represents command ID of each menu item.


### `void OnTimer( UINT_PTR nIDEvent )`

Handles [`WM_TIMER`](https://learn.microsoft.com/en-us/windows/win32/winmsg/wm-timer)
message to catch the expired timer

Following timer is used in this class:

| Timer		| Actions when expired							|
| ---		| ---									|
| `TID_INITIAL`	| Call [`PlaceWindow`](#void-placewindow-void-) to set position and size of the main frame.	|


### `void OnViewWordWrap( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_WINDOW_TILE_HORZ` in `WPARAM`
to take action for `View` menu's `Word wrap`

( The ID `ID_WINDOW_TILE_HORZ` is not concerned with 'Word wrap',
just applied as a unique identifier from the stock of predefined IDs. )

The action is mainly to destroy and create the client '*view*'
( like [`OnSysColorChange`](##void-onsyscolorchange-void-) does ).

They say that a scroll bar once created can not be deleted individually.
So the client window with the scroll bar must be destroyed and created to remove the scroll bar.
There is a guy who checked the client window of Notepad before and after 'Word wrap'.
Using [Spy++](https://learn.microsoft.com/en-us/visualstudio/debugger/using-spy-increment),
the guy found that the window handle of the client is not same as before `Word wrap`.
The guy concluded that the client window is replaced with the new one to show/hide the scroll bar.
That's why such complicated jobs are done here only to show/hide the scroll bar.


### `void OnViewMenuBar( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_WINDOW_TILE_VERT` in `WPARAM`
to take action for `View` menu's `Menu bar`

( The ID `ID_WINDOW_TILE_VERT` is not concerned with 'Menu bar',
just applied as a unique identifier from the stock of predefined IDs. )

This command is for the users who want to expand the client space even just a little bit.
The trouble is to provide a remedy to the users who regret doing that.
That may be the reason why this function is not provided generally.


### `void OnViewStatusBar( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `ID_VIEW_STATUS_BAR` in `WPARAM`
to take action for `View` menu's `Status bar`

By default, this function to show/hide the status bar is implemented somewhere in
[`CFrameWnd`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cframewnd-class).
This handler is added to save user's selection in the registry.


### `void OnUpdateCommand( CCmdUI* pCmdUI )`

Handles [`CN_UPDATE_COMMAND_UI`](https://learn.microsoft.com/en-us/cpp/mfc/reference/ccmdtarget-class#oncmdmsg)
in the range of `ID_FILE_NEW` to `ID_FORMAT_FONT`
to update the status ( enabled or disabled / checked or unchecked ) of each command in the menu

( The reason why the predefined IDs in the stock are used in this class is,
these IDs are easy to lump together in an appropriate range to use `ON_UPDATE_COMMAND_UI_RANGE` macro. )


### `LRESULT OnSupportDarkMode( WPARAM wParam, LPARAM lParam )`

Handles `WM_DARK_SUPPORT` message ( defined in this class )
to serve other windows a function to '*darken*'

This is the endpoint function when the
[`SupportDarkMode()`](#supportdarkmode-hwnd-)
macro was called.


### `LRESULT OnMessageDlg( WPARAM wParam, LPARAM lParam )`

Handles `WM_MESSAGEDLG` message ( defined in this class )
to serve other windows a function to popup a '*darkened*' message box

This is the endpoint function when the
[`MessageDlg()`](#messagedlg-lpszprompt-ntype-)
macro was called.


### `LRESULT OnIndicator( WPARAM wParam, LPARAM lParam )`

Handles `WM_INDICATOR` message ( defined in this class )
to serve other windows a function to set a pane text in the status bar

This is the endpoint function when the
[`SetIndicator()`](#setindicator-ipane-lpsztext-)
macro was called.



## Specific Functions

### `void CreateClient( void )`

Creates the client window

This function does the followings:

* Destroy and create the status bar
* Destroy and create the client '*view*'
* Initialize '*darkened*' state by calling [`CDark::Initialize`](CDark.md#void-initialize-hwnd-hwnd-).

The reason to do this 'destroy and create' is explained before.
See [`OnViewWordWrap()`](#void-onviewwordwrap-void-).


### `void PlaceWindow( void )`

Places the application to the left of the primary monitor

The window keeps 1 pixel distance from the bottom of desktop area
to keep distance from kindness of Windows&reg;
to resize the window full-height when the window touched the bottom of the desktop.
