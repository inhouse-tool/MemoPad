# `CDark`

Implemented in [`Dark.h`](../../MemoPad/MemoPad/Dark.h) / [`Dark.cpp`](../../MemoPad/MemoPad/Dark.cpp)

This class makes the application window '*darkened*' as if to support Dark Mode,
as it is `WM_PAINT` ( and other messages ) handler(s)
to draw the window ( and the controls on it ) in '*darkened*' colors.

You can '*darken*' your window with the following steps:

1. Own an instance of this class as a member of your window class.
1. Call [`Initialize( HWND hWnd )`](#void-initialize-hwnd-hwnd-) with the handle of your window
when the handle is settled in the functions like
[`OnInitDialog()`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class#oninitdialog)
or [`OnCreate( LPCREATESTRUCT lpCreateStruct )`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cwnd-class#oncreate), etc.

After this initialization, the procedures to '*darken*' your window are automatically done in this class.

If you chose '*Light*' mode by *Settings > Personalization > Colors*, your window will be drawn in 'light' colors.
( The word '*darkened*' represents
the state of being capable of changing colors according to the mode of '*Dark*' or '*Light*' hereafter. )

Note that this is not an 'official' way to support Dark Mode in MFC applications,
since Microsoft is NOT documenting how to do it in MFC applications.
( Maybe Microsoft is NOT providing the 'official' method to support Dark Mode for MFC applications... )
So this class makes your window '*darkened*' by force.

May this source be with you. Then your journey towards the Dark Mode will be complete!



## Constructor & Destructor


### `CDark( void )`

The constructor

This function initializes the members in this class.

The DLL entry point to draw the scroll bars is overridden with the one supports Dark Mode at this moment,
since the override at
[`Initialize( HWND hWnd )`](#void-initialize-hwnd-hwnd-)
is too late.
( See [`HookScrollBar()`](#void-hookscrollbar-void-) for the details. )

Registry value `Settings/EnableDark` is a 'safety switch' to disable this kind of override.
When the value is `0`, this class does nothing.
Turn it off when Microsoft has changed some specifications to make this class not to work.


### `~CDark( void )`

The destructor

This function cleans the members up.

Everything allocated should be released.
Everything created should be destroyed.
In case this class is used as a member in a modal dialog,
the destruction of this class does not always mean the end of the process.
Everything should be removed in the destructor to avoid leaks.


## Interface Functions for Application


### `void Initialize( HWND hWnd )`

The initializer

Exported method 1 of 3

This function calls [`SetDarkThemeEnable()`](#void-setdarkthemeenable-hwnd-hwnd-)
to enable 'Dark mode' to the given window,
calls [`HookWinProc()`](#void-hookwinproc-hwnd-hwnd-)
to 'hook' all subwindows on the given window,
to involve them '*darkened*' mode.

Call this function with a handle of your window
then your window will be '*darkened*'.


### `bool IsDarkMode( void )`

Returns the setting of Windows&reg; is in 'Dark Mode' or not

Exported method 2 of 3

Call this function when the application want to know the mode ( for the purpose of selecting color or something ).


### `LRESULT PopUp( WPARAM wParam, LPARAM lParam )`

Shows a popup dialog and returns the user's choice

Exported method 3 of 3

Call this function to popup '*darkened*'
[`MessageBox`](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox).

This function is designed to be called from 
[`CWinApp::DoMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cwinapp-class#domessagebox) where
[`AfxMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display#afxmessagebox)
calls as the endpoint function.
Give
[message box style](https://learn.microsoft.com/en-us/cpp/mfc/reference/styles-used-by-mfc#message-box-styles)
`UINT nType` as `wParam`,
pointer to the text `LPCTSTR lpszPrompt` as `lParam`
like below:

```
int
CWinApp::DoMessageBox( LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt )
{
	return	m_dark.PopUp( (WPARAM)nType, (LPARAM)lpszPrompt );
}
// Where 'm_dark' is an instance of 'CDark' owned by the application class.
```

This function `CDark::PopUp()` calls
[`CDarkBox::Setup()`](#void-setup-wparam-wparam-lparam-lparam-)
to build a dialog template on memory.
Then this function calls
[`CDialog::DoModal()`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class#domodal)
to popup the dialog.

The return value is compatible with
[`AfxMessageBox`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstring-formatting-and-message-box-display#afxmessagebox)
like `IDOK`, `IDCANCEL`, `IDABORT`, `IDRETRY`, `IDIGNORE`, `IDYES`, `IDNO`, `IDTRYAGAIN` and `IDCONTINUE`.

## Internal Functions for Dark Theme

### `DWORD GetWinVer( void )`

Returns version of Windows&reg; as `0`, `7`, `8`, `10`, `11`

This function is prepared to use as a safe guard from older Windows&reg;.
But not used as a result.
Instead of the guard, a notice to avoid use with ancient Windows&reg; was added in README of the application.

### `void GetDarkThemeAPIs( void )`

Gets address of ( undocumented ) APIs in `uxtheme.dll` for runtime use

The APIs loaded from `uxtheme.dll` are as follows:

| Ordinal	| Name					|
| ---:		| ---					|
| 49		| `OpenNcThemeData`			|	
| 104		| `RefreshImmersiveColorPolicyState`	|	
| 132		| `ShouldAppsUseDarkMode`		|	
| 133		| `AllowDarkModeForWindow`		|	
| 135		| `SetPreferredAppMode`			|	

These APIs are NOT documented in
["uxtheme.h header"](https://learn.microsoft.com/en-us/windows/win32/api/uxtheme/) ( as of Nov. 2023 ).
If you have installed Visual Studio 2022,
try `C:\Windows\System32>dumpbin /exports uxthme.dll` with Developer Command Prompt.
These ordinals are shown as `[NONAME]`.
( Then, where are those plausible names come from? )

Judging from the situation, Microsoft wants to keep them hidden.
Maybe the specifications of these APIs are not fixed enough.
( They say the last change was on Windows10 1903 and then not changed until Windows11 23H2. )
Or, maybe Microsoft is pushing us to insert
[WinUI](https://learn.microsoft.com/en-us/windows/apps/winui/)
layer to support Dark Mode.

If Microsoft changed the specifications of these APIs in `uxthme.dll`, this class will not work.
In such a case, turn off the 'safety switch' by writing `0` in registry value `Settings/EnableDark`.


### `void GetDarkThemeSupport( void )`

Gets whether 'Dark Mode' is supported and whether 'Dark Mode' is chosen by Windows&reg;

The DLL entry point to draw the scroll bars is overridden by this function,
since this function calls [`HookScrollBar()`](#void-hookscrollbar-void-).


### `void SetDarkThemeEnable( HWND hWnd )`

Enables 'Dark Mode' to given window

The word 'Dark Mode' means 'real' Dark Mode brought by `uxtheme.dll` in Windows&reg;.
The mode affects the title bar of the main window and the scroll bars of the client area.
That's all.
All subwindows like controls on a dialog is not affected.
Call [`HookWinProc()`](#void-hookwinproc-hwnd-hwnd-) to involve the subwindows in the '*darkened*' mode.


### `void HookScrollBar( void )`

Enables 'Dark Mode' of scroll bars

This function overrides the DLL entry point to draw the scroll bar in Common Control DLL ( `comctl32.dll` ),
with the entry point which can draw in 'Dark Mode'.


### `HTHEME HookedClassProc( HWND hWnd, LPCWSTR pszClassList )`

Returns 'Dark Mode' enabled theme handle for the scroll bars

[`HookScrollBar()`](#void-hookscrollbar-void-)
replaces the entry point of `"ScrollBar"` class of the window with this function.
( That's why this function is `static` as a callback function.)
This function returns a handle of a theme from `"ScrollBar"` class **in Explorer**.
This **Explorer** has capability to draw the scroll bars in 'Dark Mode'.
So, [`HookScrollBar()`](#void-hookscrollbar-void-)
makes the scroll bar of the given window to support 'Dark Mode' indirectly.


### `IMAGE_THUNK_DATA * FindThunk( void* pBase, char* pchDLL, WORD wOrdinal )`

Retrieves a ['thunk'](https://en.wikipedia.org/wiki/Thunk)
of given ordinal of given DLL name out of given
[PE header](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#file-headers)

This function is used to get the thunk of `OpenNcThemeData` in `uxtheme.dll` out of Common Control DLL.



## Internal Functions to Darken Controls


### `void SetColors( void )`

Selects colors and brushes according to 'Dark Mode' or 'Light Mode'


### `void HookWinProc( HWND hWnd )`

'Hook's the given window to involve it in the '*darkened*' mode

This function 'Hook's window procedure of the given window
( and the controls on it )
to override `WM_PAINT` ( and other ) message handler(s).

Technically, it is not 'Hook' but
'[SubClass](https://learn.microsoft.com/en-us/cpp/mfc/reference/cwnd-class#subclasswindow)'
used here to override the window procedures.
( If you use real
'[Hook](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexw)',
anti-virus software on your PC will catch it as doubtful behavior. )
Regard 'Hook' used here as a term to shorten the name of functions hereafter.

The controls 'hooked' here are followings.

On dialogs ( class name '`#32770`' ):

* [Button](https://learn.microsoft.com/en-us/cpp/mfc/reference/cbutton-class) ( including check box, push button, radio button, group box )
* [Combobox](https://learn.microsoft.com/en-us/cpp/mfc/reference/ccombobox-class) ( including drop list )
* [Date and Time Picker](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdatetimectrl-class)
* [Edit Control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class)
* [Static Control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstatic-class)
* [Spin Button Control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cspinbuttonctrl-class)

On other windows:

* Top [window](https://learn.microsoft.com/en-us/cpp/mfc/reference/cwnd-class) itself ( including its menu bar )
* [Status bar](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstatusbar-class)
* [Scroll bars](https://learn.microsoft.com/en-us/cpp/mfc/reference/cscrollbar-class) ( both vertical and horizontal )

Note that NOT all kind of control is supported here.
Slider, Progress bar, Tree Control, etc. are left unsupported,
since they are not used in this application.
Someday more controls may be supported to complete this class...
( The journey towards the Dark Mode is not complete yet... )


### `void HookWindow( HWND hWnd, SUBCLASSPROC pfnWnd )`

'Hook's a top window with a given procedure

If the given window is a 'real' top level window ( i.e. its parent window is 'Desktop' ),
make a UI state ( active or not / show or hide keyboard accelerator / get or lost focus ) holder
for the controls on the window.


### `void UnhookWindow( HWND hWnd )`

'Unhook's a top window

Removes 'Hook' of all controls on the given window and
remove UI state holder for the given window.


### `LRESULT OnWindow( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a top [window](https://learn.microsoft.com/en-us/cpp/mfc/reference/cwnd-class)

This function is called by Windows&reg; when a window message is sent to the top window.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgWindow` via `dwRef` which points the class instance.


### `LRESULT OnMsgWindow( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a top window

| Window Message	| Action	|
| ---			| ---		|
| `WM_CHANGEUISTATE`	| Get UI state from `wParam` and save it in UI state holder |
| `WM_CLOSE`		| Remove all 'Hook's of the window |
| `WM_CTLCOLOREDIT`	| Set '*darkened*' colors to the edit control if the application does not override them |
| `WM_CTLCOLORSTATIC`	| Set '*darkened*' colors to the static control if the application does not override them |
| `WM_ERASEBKGND`	| Fill the background in '*darkened*' background color |
| `WM_NCPAINT` `WM_NCACTIVATE`	| Draw a thin line instead of '*Annoying line*' ( 3 pixel height rectangle appears above the client area which cannot be erased easily )  |
| `WM_SYSCOLORCHANGE`	| Retake current mode of 'Dark' or 'Light', set colors again and redraw everything<sup>1</sup> |
| `WM_UAHDRAWMENU`	| Draw the menu bar in '*darkened*' background color |
| `WM_UAHDRAWMENUITEM`	| Draw the menu item in '*darkened*' colors |

<sup>1:</sup> Scroll bars are not redrawn by this class.<sup>2</sup><br>
<sup>2:</sup> Application should handle `WM_SYSCOLORCHANGE`
to destroy and create the client window to set new scroll bars.<sup>3</sup><br>
<sup>3:</sup> See [`CMainFrame::OnViewWordWrap()`](CMainFrame.md#void-onviewwordwrap-void-).


### `LRESULT OnButton( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a
[button control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cbutton-class)

This function is called by Windows&reg; when a window message is sent to the button control.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgButton` via `dwRef` which points the class instance.


### `LRESULT OnMsgButton( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a button control

| Window Message	| Action	|
| ---			| ---		|
| `WM_CTLCOLOR`		| Select '*darkened*' colors |
| `WM_ENABLE`		| Redraw the button |
| `WM_KILLFOCUS`	| Fill background with '*darkened*' color |
| `WM_LBUTTONDOWN`	| Memory as 'clicked control' |
| `WM_LBUTTONUP`	| Forget the 'clicked control' |
| `WM_PAINT`		| Call `OnPaintButton` to draw the button with '*darkened*' color  |
| `WM_UPDATEUISTATE`	| Get UI state from `wParam` and save it in UI state holder |


### `void OnPaintButton( HWND hWnd, HDC hDC, CRect rect )`

Draws a button control

This function selects a font to draw text of the button
and calls a drawer function according to the button style as below:

| Button style				| Function	|
| ---					| ---		|
| `BS_CHECKBOX` `BS_AUTOCHECKBOX`	| `OnPaintCheckBox` the function to draw a checkbox |
| `BS_PUSHBUTTON` `BS_DEFPUSHBUTTON`	| `OnPaintPushButton` the function to draw a push button |
| `BS_RADIOBUTTON` `BS_AUTORADIOBUTTON`	| `OnPaintRadioButton` the function to draw a radio button |
| `BS_GROUPBOX`				| `OnPaintGroupBox` the function to draw a groupbox |


### `void OnPaintCheckBox( HWND hWnd, HDC hDC, CRect rect )`

Draws a checkbox control

A checkbox control consists of two elements.

1. Check mark square ( round square in Windows&reg;11 )
1. Text abuts on the check mark

The check mark is drawn in the color selected by the following conditions:

* Checked or not
* 'Dark Mode' or 'Light Mode'
* 'Hot' ( mouse cursor is hovering over the control ) or not

The text is drawn with the following additions:

* After `Alt` key was hit, draw keyboard accelerator with underline.
* If the control has keyboard focus, draw a focus rectangle.


### `void OnPaintPushButton( HWND hWnd, HDC hDC, CRect rect )`

Draws a push button control

The push button is drawn in the color selected by the following conditions:

* Pushed or not ( `hWnd` equals to the 'clicked control' or not )
* 'Dark Mode' or 'Light Mode'
* 'Hot' ( mouse cursor is hovering over the control ) or not

The text is drawn with the following additions:

* After `Alt` key was hit, draw keyboard accelerator with underline.
* If the control has keyboard focus, draw a focus rectangle.


### `void OnPaintRadioButton( HWND hWnd, HDC hDC, CRect rect )`

Draws a radio button control

A radio button control consists of two elements.

1. Radio button circle
1. Text abuts on the radio button

The radio button is drawn in the color selected by the following conditions:

* Checked or not
* 'Dark Mode' or 'Light Mode'
* 'Hot' ( mouse cursor is hovering over the control ) or not

The text is drawn with the following additions:

* After `Alt` key was hit, draw keyboard accelerator with underline.
* If the control has keyboard focus, draw a focus rectangle.


### `void OnPaintGroupBox( HWND hWnd, HDC hDC, CRect rect )`

Draws a group box control

The box is drawn in the color selected by the following condition:

* 'Dark Mode' or 'Light Mode'

The group box in Windows&reg;11 is drawn with a round rectangle.


### `LRESULT OnComboBox( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a
[combobox control](https://learn.microsoft.com/en-us/cpp/mfc/reference/ccombobox-class)

This function is called by Windows&reg; when a window message is sent to the combobox.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgOnComboBox` via `dwRef` which points the class instance.


### `LRESULT OnMsgComboBox( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a combobox

| Window Message	| Action	|
| ---			| ---		|
| `WM_CTLCOLOREDIT`<br>`WM_CTLCOLORLISTBOX`	| Select '*darkened*' colors of a combobox |
| `WM_KILLFOCUS`	| Redraw the combobox |
| `WM_LBUTTONDOWN`	| Memory as 'clicked control' |
| `WM_LBUTTONUP`	| Forget the 'clicked control' |
| `WM_PAINT`		| Call `OnPaintComboBox` to draw the combobox with '*darkened*' color  |
| `CB_SETCURSEL`	| Call `OnPaintComboBox` to draw the combobox with '*darkened*' color with 'selected' status  |


### `void OnPaintComboBox( HWND hWnd, HDC hDC, CRect rect )`

Draws a combobox control

The combobox is drawn in the color selected by the following conditions:

* 'Dark Mode' or 'Light Mode'
* 'Hot' ( mouse cursor is hovering over the control ) or not

If the style of the combobox is 'Dropdown list' ( `CBS_DROPDOWNLIST` ),
the text inside the combobox must be drawn here.
The text is drawn with the following addition:

* If the control has keyboard focus, draw a focus rectangle.

If the style of the combobox is 'Dropdown' ( `CBS_DROPDOWN` ),
the text inside the combobox will be drawn by edit control handler.
So it is not needed to be drawn here.


### `LRESULT OnDateTime( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a
[date and time picker control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdatetimectrl-class)

This function is called by Windows&reg; when a window message is sent to the date and time picker control.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgDateTime` via `dwRef` which points the class instance.


### `LRESULT OnMsgDateTime( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a date and time picker

| Window Message	| Action	|
| ---			| ---		|
| `WM_ERASEBKGND`	| Do nothing ( to avoid being erased with 'light' color ) |
| `WM_LBUTTONDOWN`	| Call `OnClickDateTime` to show 'selected control' |
| `WM_KILLFOCUS`	| Forget 'selected control' |
| `WM_PAINT`		| Draw the control with '*darkened*' color |
| `WM_UPDATEUISTATE`	| Invalidate the spin control to draw its updated UI state |

Note that this function is not completed yet.
When the numeric keys are hit on the control,
they are not shown on the control until all digits are set.
Anyway, the calendar popups under a date picker is not '*darkened*' by this class.


### `void OnClickDateTime( HWND hWnd, WPARAM wParam, LPARAM lParam )`

Draws a focus rectangle on the date and time picker control


### `LRESULT OnEdit( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a
[edit control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cedit-class)

This function is called by Windows&reg; when a window message is sent to the edit control.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgEdit` via `dwRef` which points the class instance.


### `LRESULT OnMsgEdit( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a edit control

| Window Message	| Action	|
| ---			| ---		|
| `WM_CTLCOLOR`		| Select '*darkened*' colors |
| `WM_ERASEBKGND`	| Fill the background with '*darkened*' background color |
| `WM_KEYDOWN`		| Redraw the control ( with new character ) |
| `WM_KILLFOCUS`	| Redraw the control ( with new UI state ) |
| `WM_NCPAINT`		| Do nothing ( to avoid flicker ) |
| `WM_PAINT`		| Draw as usual then draw a frame for a spin control |


### `LRESULT OnStatic( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a
[static control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstatic-class)

This function is called by Windows&reg; when a window message is sent to the static control.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgStatic` via `dwRef` which points the class instance.


### `LRESULT OnMsgStatic( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a static control

| Window Message	| Action	|
| ---			| ---		|
| `WM_ERASEBKGND`	| Fill the background with '*darkened*' background color |
| `WM_PAINT`		| Call `OnPaintStatic` to draw the static with '*darkened*' color |
| `WM_SETTEXT `		| Redraw the control ( with new text ) |


### `void OnPaintStatic( HWND hWnd, HDC hDC, CRect rect )`

Draws a static control

The text is drawn with the following addition:

* After `Alt` key was hit, draw keyboard accelerator with underline.


### `LRESULT OnSpin( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a
[spin button control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cspinbuttonctrl-class)

This function is called by Windows&reg; when a window message is sent to the spin button control.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgSpin` via `dwRef` which points the class instance.


### `LRESULT OnMsgSpin( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a spin button control

| Window Message	| Action	|
| ---			| ---		|
| `WM_ERASEBKGND`	| Fill the background with '*darkened*' background color |
| `WM_PAINT`		| Call `OnPaintSpin` to draw the static with '*darkened*' color |


### `void OnPaintSpin( HWND hWnd, HDC hDC, CRect rect )`

Draws a spin button control


### `LRESULT OnStatusBar( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a
[status bar](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstatusbar-class)

This function is called by Windows&reg; when a window message is sent to the status bar.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgStatusBar` via `dwRef` which points the class instance.


### `LRESULT OnMsgStatusBar( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a status bar

| Window Message	| Action	|
| ---			| ---		|
| `WM_CTLCOLOREDIT` `WM_CTLCOLORLISTBOX`	| Select '*darkened*' colors |
| `WM_ERASEBKGND`	| Fill the background with '*darkened*' background color |
| `WM_PAINT`		| Call `OnPaintStatusBar` to draw the status bar with '*darkened*' color |
| `WM_SIZE`		| Redraw the status bar ( with new size ) |


### `void OnPaintStatusBar( HWND hWnd, HDC hDC, CRect rect )`

Draws a status bar

The text in each pane is drawn with the horizontal alignment as below:

| `Left-aligned`	| `\tCenter-aligned\t`	| `\tRight-aligned`	|
| :---			| :---:			| ---:			|
| Left-aligned		| Center-aligned	| Right-aligned		|


### `LRESULT OnScrollBars( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uId, DWORD_PTR dwRef )`

Handles window messages to a
[scroll bar](https://learn.microsoft.com/en-us/cpp/mfc/reference/cscrollbar-class)

This function is called by Windows&reg; when a window message is sent to the scroll bar.
This is a static callback function, not a dynamically allocated class member method.
So, this function calls `OnMsgScrollBars` via `dwRef` which points the class instance.


### `LRESULT OnMsgScrollBars( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )`

Handles the following window messages to a scroll bar

| Window Message	| Action	|
| ---			| ---		|
| `WM_NCPAINT`		| Call `FillGap` to fill gap between vertical scroll bar and horizontal scroll bar |


### `LRESULT OnCtlColorEdit( HWND hWnd, WPARAM wParam )`

Sets colors to an edit control in a combobox and to a pane in the status bar

Both are drawn in the same color in this class.


### `void FillGap( HWND hWnd )`

Fills the gap between vertical and horizontal scroll bars

In case of a window with both vertical and horizontal scroll bars,
a square gap appears on the right-bottom edge in Dark Mode.
This function is prepared to fill the gap.


### `void GetWndTabAlt( HWND hWnd, bool& bTab, bool& bAlt )`

Gets 'Tab' and 'Alt' flags from UI state

Meanings of the flags are:

| Flag	| If set	| Then do	|
| ---	| ---		| ---		|
| Tab	| Focus was moved by `Tab` key.	| Show a focus rectangle on the controls	|
| Alt	| `Alt` key was hit.		| Show keyboard accelerator on the controls and the menu items |


### `DWORD GetWndUIState( HWND hWnd )`

Gets UI state of the given window

UI state ( 'Tab' and 'Alt' ) is the property of a top window.
All controls on a window apply the UI state of the top window.
This class holds multiple UI states to handle multiple top windows
so that one instance of this class can handle multiple top windows.

### `void SetWndUIState( HWND hWnd, DWORD dwUIState )`

Set UI state of the given window.

Store the UI state of the given window for further reference by `GetWndUIState`.



## Internal Functions for GDI+


### `void GdipRoundRect( HDC hDC, CRect rect, CPoint ptEdge, COLORREF crInner, COLORREF crEdge )`

Draws a round rectangle ( of push buttons ) by GDI+

If the round rectangle is drawn by
[CDC](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class) method, it will be jaggy.
So it is drawn by
[GDI+ method](https://learn.microsoft.com/en-us/windows/win32/api/gdiplusgraphics/nl-gdiplusgraphics-graphics)
in this class.


### `void GdipCircle( HDC hDC, CRect rect, int nThickness, COLORREF crInner, COLORREF crEdge )`

Draws a circle ( in a radio button ) by GDI+

If the circle is drawn by
[CDC](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class) method, it will be jaggy.
So it is drawn by
[GDI+ method](https://learn.microsoft.com/en-us/windows/win32/api/gdiplusgraphics/nl-gdiplusgraphics-graphics)
in this class.

### `void GdipCheckMark( HDC hDC, CRect rect, COLORREF crMark )`

Draws a check mark ( in a checkbox ) by GDI+

If the mark is drawn by
[CDC](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class) method, it will be jaggy.
So it is drawn by
[GDI+ method](https://learn.microsoft.com/en-us/windows/win32/api/gdiplusgraphics/nl-gdiplusgraphics-graphics)
in this class.


### `void GdipArrow( HDC hDC, CRect rect, COLORREF crMark, bool bUp )`

Draws a 'arrow' ( in a combobox ) by GDI+

If the 'arrow' is drawn by
[CDC](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdc-class) method, it will be jaggy.
So it is drawn by
[GDI+ method](https://learn.microsoft.com/en-us/windows/win32/api/gdiplusgraphics/nl-gdiplusgraphics-graphics)
in this class.




# `CDarkBox`

Implemented in [`Dark.h`](../../MemoPad/MemoPad/Dark.h) / [`Dark.cpp`](../../MemoPad/MemoPad/Dark.cpp)


This class makes '*darkened*'
[MessageBox](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox).

[MessageBox](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox)
does not require dialog resources, since it build a dialog template on memory dynamically.
This class does similar and shows similar. But the dialog shown by this class is '*darkened*'.



## Interface Functions for Application



### `void SetUp( WPARAM wParam, LPARAM lParam )`

Builds a dialog template like
[MessageBox](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox)

This function is called from
[`CDark::PopUp()`](#lresult-popup-wparam-wparam-lparam-lparam-).


Building the dialog template is done in the following steps:

1. Count bytes for the template.
1. Allocate a memory buffer for the template.
1. Fill the memory buffer with the template items.

With these steps, the template is built like this:

| `DLGTEMPLATE`				|
| :---					|
| `DLGITEMTEMPLATE` for icon		|
| `DLGITEMTEMPLATE` for text		|
| `DLGITEMTEMPLATE` for button #1	|
| `DLGITEMTEMPLATE` for button #2	|


Finally,
[`InitModalIndirect`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class#initmodalindirect)
is called to initialize a dialog with this template.

Then the dialog is ready to call
[`DoModal()`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class#domodal)
as
[`CDark::PopUp()`](#lresult-popup-wparam-wparam-lparam-lparam-)
is doing so.

## Constructor & Destructor


### `CDarkBox( void )`

Does initialization of some members

This function is called from
[`CDark::PopUp()`](#lresult-popup-wparam-wparam-lparam-lparam-).


### `~CDarkBox( void )`

Frees memory buffer used for dialog template

This function is called from
[`CDark::PopUp()`](#lresult-popup-wparam-wparam-lparam-lparam-) implicitly.


## Overridden Functions


### `BOOL OnInitDialog( void )`

Initializes the dialog

Almost same as usual
[`CDialog::DoModal()`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class#domodal)
except to call
[`CDark::Initialize()`](#void-initialize-hwnd-hwnd-)
to make the dialog '*darkened*'.



## Message Handlers


### `void OnShowWindow( BOOL bShow, UINT nStatus )`

Show or hide the window

Almost same as usual
[`CDialog::OnShowWindow()`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cwnd-class#onshowwindow)
except to set focus on the default button.


## Specific Functions


### `DWORD FillDialog( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam )`

Fills the top block of the dialog template

This function is called from [`SetUp()`](#void-setup-wparam-wparam-lparam-lparam-) 2 times.
1st is to count bytes of memory, 2nd is to fill the memory.


### `DWORD FillIcon( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam )`

Fills the icon block of the dialog template

This function is called from [`SetUp()`](#void-setup-wparam-wparam-lparam-lparam-) 2 times.
1st is to count bytes of memory, 2nd is to fill the memory.

The icon was select by [`SelectIcon()`](#lpwstr-selecticon-uint-uicon-).
If a valid icon is not selected, this block is omitted.


### `DWORD FillText( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam )`

Fills the text block of the dialog template

This function is called from [`SetUp()`](#void-setup-wparam-wparam-lparam-lparam-) 2 times.
1st is to count bytes of memory, 2nd is to fill the memory.

If the text is too long to set in the default size ( 240 du x 64 du; where 'du' is 'dialog unit' ) of this popup,
the popup is enlarged enough to contain all text.


### `DWORD FillButtons( BYTE* pb, DLGTEMPLATE* pTemp, WPARAM wParam, LPARAM lParam )`

Fills the button blocks of the dialog template

This function is called from [`SetUp()`](#void-setup-wparam-wparam-lparam-lparam-) 2 times.
1st is to count bytes of memory, 2nd is to fill the memory.

The number of the buttons, IDs and texts are selected by
[`SelectButtons()`](#int-selectbuttons-uint-utype-uint-puidbutton-cstring-pstrbutton-).


### `int SelectButtons( UINT uType, UINT* puIdButton, CString* pstrButton )`

Selects the buttons to set according to `nType`

This function is called from the methods in this class internally.

Control IDs of the buttons are stored in `*puIdButton` and
the texts to show on the buttons are stored in `*pstrButton` as below:

| `nType`		| `*puIdButton`					| `*pstrButton`				|
| ---			| ---						| ---					|
| `MB_OK`		| `IDOK`					| `"OK"`				|
| `MB_OKCANCEL`		| `IDOK` `IDCANCEL`				| `"OK"` `"Cancel"`			|
| `MB_ABORTRETRYIGNORE`	| `IDABORT` `IDRETRY` `IDIGNORE`		| `"&Abort"` `"&Retry"` `"&Ignore"`	|
| `MB_YESNOCANCEL`	| `IDYES` `IDNO` `IDCANCEL`			| `"&Yes"` `"&No"` `"Cancel"`		|
| `MB_YESNO`		| `IDYES` `IDNO`				| `"&Yes"` `"&No"`			|
| `MB_RETRYCANCEL`	| `IDRETRY` `IDCANCEL`				| `"&Retry"` `"Cancel"`			|
| `MB_CANCELTRYCONTINUE`| `IDCANCEL` `IDTRYAGAIN` `IDCONTINUE`		| `"Cancel"` `"&Tray Again"` `"&Continue"`|
| Others		| `IDOK`					| `"OK"`		|

Return value is the number of buttons set in `*puIdButton` and `*pstrButton`.


### `LPWSTR SelectIcon( UINT uIcon )`

Selects the icon to show according to `uIcon`

This function is called from the methods in this class internally.

Return value `LPWSTR` is not a pointer to a string
but a `MAKEINTRESOURCE` value representing a resource ID.

`uIcon` specifies one of the below:

| `uIcon`		| Return value		| Image of the icon		|
| ---			| ---			| ---				|
| `MB_ICONHAND`	`MB_ICONSTOP`	| `IDI_HAND`		| A stop-sign icon		|
| `MB_ICONQUESTION`	| `IDI_QUESTION`	| A question-mark icon		|
| `MB_ICONEXCLAMATION` `MB_ICONWARNING`	| `IDI_EXCLAMATION`	| An exclamation-point icon	|
| `MB_ICONASTERISK` `MB_ICONASTERISK`	| `IDI_ASTERISK`	| An icon consisting of a lowercase letter i in a circle	|
| `MB_USERICON`		| `NULL`	| ( Not supported )	|
| `MB_ICONMASK`		| `IDR_MAINFRAME`	| An icon of the application	|

The last one is an original extension of this class to show the icon of the application.
Note that is not compatible with
[MessageBox](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox).


### `BYTE * Align( BYTE* pb )`

Returns a pointer to the next 4byte-boundary

This alignment is required for the dialog template.

This function is called from the methods in this class internally.

