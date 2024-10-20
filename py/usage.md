sudo xattr -rd com.apple.quarantine ./

sudo codesign --force --deep --sign - /Users/ws/Documents/project/develop_mac_v8.0/Debug/Wondershare\ DemoCreator\ 8.app

python3 dylib_copy.py  ../Debug ../Debug/Wondershare\ DemoCreator\ 8.app/Contents/MacOS

python3 ts.py  ../Src/PDemoCreator/EasyEditing easyediting_ja
python3 ts.py  ../Src/PDemoCreator/EasyEditing

python3 qm.py  ../Src/PDemoCreator/EasyEditing easyediting_ja
python3 qm.py  ../Src/PDemoCreator/EasyEditing

python3 qm_copy.py  ../Src/PDemoCreator/EasyEditing ../Resource/Mac/Bin/lang
python3 dir_copy.py  ../Resource/Mac/Bin/lang ../Debug/Wondershare\ DemoCreator\ 8.app/Contents/MacOS/DemoCreatorEditor\ 8.app/Contents/Resources/lang
python3 dir_copy.py  ../Resource/Mac/Bin/lang ../Debug/Wondershare\ DemoCreator\ 8.app/Contents/MacOS/DemoCreator\ Record\ 8.app/Contents/Resources/lang
python3 dir_copy.py  ../Resource/Mac/Bin/lang ../Debug/Wondershare\ DemoCreator\ 8.app/Contents/Resources/lang