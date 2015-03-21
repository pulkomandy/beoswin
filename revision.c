/*

  Revisions to this application.

1.00 (6-Mar-01) 		
- First Release

1.01 (6-Mar-01)		
- Immediate update ; can't configure Explorer as shell as forgot to put it on the menu :(

1.02 (6-Mar-01)
- Added "DontResizeScreenArea" so the resetting of the available screen area can be changed.
- DrawFilenameIcon() doesn't crash if the icon doesn't exist, does a default instead.
- Can delete shortcuts by dragging them onto the desktop bar.
- If no shortcuts found will copy them from Windows\Desktop and place them neatly.
- Shortcuts loaded and saved from/to parameter area.
- Show and Hide now actually show and hide, not minimise and restore.
- Single function call for running via ShellExecute()
- Can create shortcuts by dropping a single file onto the desktop bar. Don't like this
  but no fudges in this solution.

1.03 (7-Mar-01)
- Hidden windows that exist when the application started are not displayed in the task menu
- Changed default text font a little bit.
- Start Up Directory Execution implemented (when Explorer.Exe not running)
- Selected shortcut icons now change colour on clicking
- Can't close via ALT+F4
- Realised "tr***er" is actually "deskbar". Rats.

*/
