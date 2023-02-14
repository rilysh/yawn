## yawn
A template-based package manager

## Usage
- Create a template, for example, see [this](./test/template)
- Provide the package name. Use the file extension to separate it from the rest of others
- Provide the URL (from where you want first to download this package)
- Description and Author, both sections are optional

### Template
```
NAME=<PACKAGE_NAME_WITH_EXTENSION>
URL=<DOWNLOAD_URL>
DESC=<DESCRIPTION>
AUTHOR=<CREATOR/MAINTAINER>
```
An example template should look like this
```
NAME=example.appimage
URL=https://example.com/example.appimage.zl
DESC=This thing can do...
AUTHOR=Ghost
```

## Note
This is a very dumb package manager or more appropriately, it's not even a package manager, all it does is download an archive from the Internet and extract them to "/opt". Don't use this, except to play.
