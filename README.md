# WonderDroid

WonderDroid is an emulator of the Bandai Wonderswan and Wonderswan Color handheld game consoles for Android. This emulator was originally written by [Daniel Palmer](https://github.com/danielgpalmer) for older versions of Android and was later imported from [Google Code](https://code.google.com/p/wonderdroid). I have forked his project to bring it up to date and fix bugs.

## How to use it

  1. Open WonderDroid at least once so that it can create the directories for ROMs, etc.
  1. Copy your game ROMs in either zip archives or plain .wsc or .ws files into the `wonderdroid` directory* that has been automatically created. Zips with multiple files should work but it's not been tested.
  1. Open WonderDroid again, it should now have a gallery of the roms you put into it's directoy. Wonderdroid uses the metadata in the rom to select a screenshot. If a game has a question mark it will probably still work. It just hasn't been screenshot'ed yet

(*) On older phones with minimal internal storage, this directory may be located on your SD card.

## 使い方

  1. インストールしたあと、一回ワンダードロイドを開けてください。そうすれば自動にsdカードに必要なフォルダが作られます。
  1. 内部ディスク/SDカードにあるwonderdroidというフォルダにロムファイルをコピーしてください。.zipと.wscと.wsが使えます。
  1. ワンダードロイドのフォルダにロムファイルをコピーしたあと、またワンダードロイドを開けたら、フォルダに入ってるゲームがメニューに出ます。好きなゲームを選んでクリックすれば遊べます。

## Build notes

1. You will need Eclipse (tested and working on Neon) with ADT and NDK (tested and working on release 10e).
2. Download [ActionBarSherlock](http://actionbarsherlock.com), then import the project into Eclipse.
3. Clone this repo, then import the wonderdroid project into Eclipse.
4. Fix the ActionBarSherlock dependency by right-clicking on the WonderDroid project in Package Explorer, then Properties, then Android.
