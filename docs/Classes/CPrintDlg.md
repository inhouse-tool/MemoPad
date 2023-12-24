# `CPrintDlg`

Implemented in [`PrintDlg.h`](../../MemoPad/MemoPad/PrintDlg.h) / [`PrintDlg.cpp`](../../MemoPad/MemoPad/PrintDlg.cpp)

This class shows 'the print dialog' to print out the current content.

Unlike the
[standard print dialog in MFC](https://learn.microsoft.com/en-us/cpp/mfc/reference/cprintdialog-class),
this dialog is drawn in '*darkened*' color like below.

<picture>
<source media="(prefers-color-scheme: dark)" srcset="PrintInTheDark.png">
<source media="(prefers-color-scheme: light)" srcset="PrintInTheLight.png">
<img alt="Snapshot" src="PrintInTheLight.png">
</picture>

<p></p>

This dialog is just a 'human interface' to specify how to print out.
Actual job to print the content is
[done in the '*view*'](CMemoView.md#bool-print-cprintparam-param-).

## Constructor


### `CPrintDlg( CWnd* pParent )`

The constructor

This function does nothing except the construction of a
[`CDialog`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class).



## Overridden Functions


### `BOOL OnInitDialog( void )`

Initializes the dialog

This method is to override the initialization of the
[`CDialog`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class)
class.

Overridden here to insert the following procedures:

1. Call [`SupportDarkMode()`](CMainFrame.md#supportdarkmode-hwnd-) to '*darken*' the dialog
1. Load the settings from registry
1. Launch [`EnumPrintersTh()`](#uint-enumprintersth-lpvoid-pparam-) to fill the list of `Printer`
1. Select the default printer out of the list
1. Select `mm size` or `inch size`
1. Select the default paper size
1. Select `Portrait` or `Landscape`
1. Check `All` pages as default

The settings loaded at 2. are in
`\HKEY_CURRENT_USER\Software\In-house Tool\MemoPad\Print`
as follows:

| Value name	| Type		| Content			| Default value			|
| ---		| ---		| :---:				| ---:				|
| `"Printer"`	| `REG_SZ`	| Printer to output		| `"Microsoft Print to PDF"`	|
| `"Inch"`	| `REG_DWORD`	| `inch size` or not		| `0` ( `mm` )			|
| `"Size"`	| `REG_DWORD`	| Paper size			| `DMPAPER_A4`			|
| `"Landscape"`	| `REG_DWORD`	| `Landscape` or not		| `0`  ( `Portrait` )		|
| `"Header"`	| `REG_SZ`	| Header text			| ( empty )			|
| `"Footer"`	| `REG_SZ`	| Footer text			| ( empty )			|
| `"MarginL"`	| `REG_DWORD`	| Left margin			| `20` [mm]			|
| `"MarginR"`	| `REG_DWORD`	| Right margin			| `20` [mm]			|
| `"MarginT"`	| `REG_DWORD`	| Top margin			| `25` [mm]			|
| `"MarginB"`	| `REG_DWORD`	| Bottom margin			| `25` [mm]			|
| `"Tab"`	| `REG_DWORD`	| Tab count			| `8` [characters]		|

Note that there is no GUI to set tab count in this class. Use regedit if you insist on customizing the value.


### `BOOL DestroyWindow( void )`

Does specified procedures before the window is destroyed

Overridden here to terminate the worker thread of [`EnumPrintersTh()`](#uint-enumprintersth-lpvoid-pparam-).

This function terminates the thread and closes the handle opened by the thread.
This procedure prevents resource leakages when this modal dialog was closed.

This function calls
[`TerminateThread()`](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-terminatethread)
to terminate the thread
( since the thread cannot accept a signal to finish the thread
( since the thread is in deep inside of Win32 API ) ).
But the Visual Studio automatically raises
[warning C6258](https://learn.microsoft.com/en-us/cpp/code-quality/c6258)
when the compiler detected the use of
[`TerminateThread()`](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-terminatethread).
To suppress this annoying warning, this function is enclosed with
`#pragma warning( disable : 6258 )`.

But it is not good to ignore
[warning C6258](https://learn.microsoft.com/en-us/cpp/code-quality/c6258)
unilaterally.
This thread is designed to suppress resource leakages as not to allocate class instances in the thread.
These instances are allocated as member variables of this class
so that the destructor of this class can deallocate them.


### `BOOL OnOK( void )`

Acts on `OK` button

This method is to override the actions when the user clicks the `OK` button.

Overridden here to insert the following procedures:

1. If `Pages` is `Select:`, retrieve the text from the edit box and encode it by
[`GetPages()`](#bool-getpages-cstring-strpages-)
1. Retrieve the text from the `Printer` combo box and set it into the 'print parameter'
1. Set or clear 'inch' selector in the 'print parameter' according to `mm size` / `inch size` radio button
1. Set or clear 'landscape' selector in the 'print parameter' according to the `Portrait` / `Landscape` radio button
1. Save the changed settings into the registry

At 5., only changed values are saved.
So, a value never changed from the default has no entry in the registry.
They say that writing so frequently this kind of non-volatile data is not good
for your ( NAND flash-based ) SSD.


### `BOOL PreTranslateMessage( MSG* pMsg )`

Does specified procedures before window messages are dispatched

Overridden here to ignore the characters not concerned with page number entry.



## Message Handlers


### `void OnSelPrinters( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `CBN_SELCHANGE` in `WPARAM`
to take action for the `Printer` selection

The action is to set the status of the printer.


### `void OnRadioSize( UINT uID )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `BN_CLICKED` in `WPARAM`
to take action for `mm size` / `inch size` selection

The action is to set or clear
'inch' selector in the 'print parameter' according to `mm size` / `inch size` radio button.


### `void OnRadioOrientation( UINT uID )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `BN_CLICKED` in `WPARAM`
to take action for `Portrait` / `Landscape` selection

The action is to set or clear
'landscape' selector in the 'print parameter' according to `Portrait` / `Landscape` radio button.


### `void OnSelSize( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `CBN_SELCHANGE` in `WPARAM`
to take action for the paper size selection

The action is to call [`UpdatePaperSize()`](#void-updatepapersize-bool-bwhole-)
to replace the description under the combo box.


### `void OnClickMargins( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `BN_CLICKED` in `WPARAM`
to take action when `Margins` button is clicked

The action is to show [the margin dialog](#cmargindlg)
and get the settings.


### `void OnFocusPages( void )`

Handles [`WM_COMMAND`](https://learn.microsoft.com/en-us/windows/win32/menurc/wm-command)
message with `EN_SETFOCUS` in `WPARAM`
to take action when `Select:` edit box get focus

The action is to check `Select:` and uncheck `All`
as a little act of kindness to the users who want to specify pages but forgot to check `Select:`.



## Specific Functions


### `void UpdatePaperSize( bool bWhole )`

Updates the list of paper size

If the argument `bWhole` is true,
this function renews the whole of the list of paper size.<br>
If the argument `bWhole` is false,
this function renews the explanation text of paper size.


### `void UpdateOrientation( void )`

Updates the orientation 'icon'

The 'icon' is an image below the radio button of `Portrait`.
This function updates the shape of 'icon' taller for `Portrait` or broader for `Landscape` as below.

| Portrait	| Landscape	|
| :---:		| :---:		|
| <picture><source media="(prefers-color-scheme: dark)" srcset="PortraitInTheDark.png"><source media="(prefers-color-scheme: light)" srcset="PortraitInTheLight.png"><img alt="Snapshot" src="PortraitInTheLight.png"></picture>|<picture><source media="(prefers-color-scheme: dark)" srcset="LandscapeInTheDark.png"><source media="(prefers-color-scheme: light)" srcset="LandscapeInTheLight.png"><img alt="Snapshot" src="LandscapeInTheLight.png"></picture>|

Technically, this is not an icon but a frame drawn by
[static control](https://learn.microsoft.com/en-us/cpp/mfc/reference/cstatic-class).
This function sets position of this static control and replaces the 'dots' in it.


### `bool GetPages( CString strPages )`

Gets the selected pages

This function gets the pages specified in `Select:` edit box,
encode them into integers and set them into the 'print parameter'.

The text is encoded like below:

| Text		| Integers		|
| ---		| ---			|
| `"2 3 5 7"`	| `2` `3` `5` `7`	|
| `"1,2, 4,6"`	| `1` `2` `4` `6`	|
| `"2 - 6"`	| `2` `0` `6`		|

This function returns false, if the encoding was failed.

These integers are read by
[`CMemoeView::IsPageToPrint()`](CMemoView.md#bool-ispagetoprint-uint-upage-cuintarray-uapages-).


### `UINT EnumPrintersTh( LPVOID pParam )`

Thread entry function to enumerate printers

This function does the followings:

1. Call [`ListPrinters()`](#void-listprinters-void-) to list existing printers
1. Call [`CheckPrinters()`](#void-checkprinters-void-) to check status of each printer

Above procedure may take long ( in case of remote printers ).
So the procedure is executed in an independent thread not to block the GUI.

### `void ListPrinters( void )`

Makes a list of printers

This function enumerates the printers and put them into the list of `Printer`.

The enumeration is done by
[`EnumPrinters()`](https://learn.microsoft.com/en-us/windows/win32/printdocs/enumprinters)
for local and remote printers.
This function enumerates the local printers first and the remote printers next.
But the printers in the combo box are shown in alphabetical order.

This enumeration of the remote printers may take 50 seconds or more
when the client PC was just rebooted and existence of the remote PCs are not cached yet.
While this enumeration is in progress, the status of the printer is shown as `Searching printers...`
which is the default text in the dialog resource.


### `void CheckPrinters( void )`

Checks the status of printers

This function checks the status of each printer in the list of `Printer` combo box.

The status is retrieved by [`GetPrinter()`](https://learn.microsoft.com/en-us/windows/win32/printdocs/getprinter)
and 
[`OpenPrinter2()`](https://learn.microsoft.com/en-us/windows/win32/printdocs/openprinter2)
with the option flag `PRINTER_OPTION_NO_CACHE` to get the **current** status.
Because of this arrangement, this dialog is prevented from selecting a print server which is down.
In spite of this arrangement, it seems that the status is not always accurate
( ex. 'Out of Paper' is not detected ).
Maybe this accuracy depends on the environment.

This check of a remote printer may take 80 seconds or more
when the print server is down.
While this check is in progress, the status of the printer is shown as `Connecting...`
which is the text left by [`ListPrinters()`](#void-listprinters-void-).
When this check is completed, the status is renewed as `Ready` or some other text describes the status.
When this check is failed, the status is renewed as `Offline`.

You can close this dialog by clicking the red &#x2715; on the top right corner of this dialog anytime,
since the check is executed in a worker thread separated from the GUI thread.
See [`DestroyWindow()`](#bool-destroywindow-void-)
for how resource leakages are suppressed.

### `CString GetItemFromPath( CString strPath )`

Gets 'Item text' from 'Path text'

The 'Item text' is a text shown in the `Printer` combo box and
the 'Path text' is a text used in Win32 APIs.

Both text are identical for local printers.
Both text are as follows for remote printers:

| 'Item text'			| 'Path text'	|
| ---				| ---			|
| `<Device> on <Server>`	| `\\<Server>\<Device>`	|

[`ListPrinters()`](#void-listprinters-void-) retrieves the 'Path text'.
The text is converted into the 'Item text' by this function
and set into the list of `Printer` combo box,
since Windows&reg; uses the representation like 'Item text' on GUI.

This function is called from
[`OnInitDialog()`](#bool-oninitdialog-void-)
to select default printer in `Printer` combo box, from
[`ListPrinters()`](#void-listprinters-void-) to set an enumerated printer with 'Item text' in `Printer` combo box.


### `CString GetPathFromItem( CString strItem )`

Gets 'Path text' from 'Item text'

This function does the reverse of
[`GetItemFromPath()`](#cstring-getitemfrompath-cstring-strpath-).

This function is called from
[`CheckPrinters()`](#void-checkprinters-void-) to open a printer in `Printer` combo box, from
[`OnOK( void )`](#bool-onok-void-) to return the 'Path text' as an answer from this dialog.
The answer must be a 'Path text' since the '*view*' uses the text to open the printer with Win32 API.


### `int GetComboIndexByItemText( CComboBox* pCombo, CString strItem )`

Gets the index of given text in the combobox

This function searches the given text in the given combo box, returns the index of item with the text.
This function is similar to
[`CComboBox::SelectString()`](https://learn.microsoft.com/en-us/cpp/mfc/reference/ccombobox-class#selectstring)
but does not select the item.



# `CMarginDlg`

Implemented in [`MarginDlg.h`](../../MemoPad/MemoPad/MarginDlg.h) / [`MarginDlg.cpp`](../../MemoPad/MemoPad/MarginDlg.cpp)

This class shows 'the margin dialog'.

This dialog is shown like below as a modal dialog to set the margins.

<picture>
<source media="(prefers-color-scheme: dark)" srcset="MarginInTheDark.png">
<source media="(prefers-color-scheme: light)" srcset="MarginInTheLight.png">
<img alt="Snapshot" src="MarginInTheLight.png">
</picture>

<p></p>

The purpose of this dialog is to make each parameter easier to grasp visually.


## Constructor

This function does nothing except the construction of a
[`CDialog`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class).



## Overridden Functions


### `BOOL OnInitDialog( void )`

Initializes the dialog

This method is to override the initialization of the
[`CDialog`](https://learn.microsoft.com/en-us/cpp/mfc/reference/cdialog-class)
class.

Overridden here to insert the following procedures:

1. Call [`SupportDarkMode()`](CMainFrame.md#supportdarkmode-hwnd-) to '*darken*' the dialog
1. Set the text of the header in the 'print parameter' into the `Header` edit box
1. Set the text of the footer in the 'print parameter' into the `Footer` edit box
1. Set the margin values in the 'print parameter' into each edit box


### `BOOL OnOK( void )`

Acts on `OK` button

This method is to override the actions when the user clicks the `OK` button.

Overridden here to insert the following procedures:

1. Retrieve the text from the `Header` edit box and set it into the 'print parameter'
1. Retrieve the text from the `Footer` edit box and set it into the 'print parameter'
1. Retrieve the text from the edit box of each margin value and set it into the 'print parameter'


### `BOOL PreTranslateMessage( MSG* pMsg )`

Does specified procedures before window messages are dispatched

Overridden here to ignore the characters not concerned with margin entry.

This function does nothing for `Header` and `Footer` edit boxes,
for other edit boxes ( to enter the figures ) ignore the characters other than `0` &#x301c; `9` and `.`.

## Specific Functions

### `int GetValue( UINT uID )`

Gets the value of [mm]

This function gets the value from the edit box with given control.
Unit of the value is [mm].

### `void SetValue( UINT uID, int nValue )`

Sets the value of [mm]

This function sets the value into the edit box with given control.
Unit of the value is [mm].
