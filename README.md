# cppdiscogs
C++ interface to Discogs API: https://www.discogs.com/developers/#

Currently, just a shell tool that lists a users's releases (with some sort criteria).

Compile:
```
mkdir build
cd build
cmake ../
make
```

Run examples:

To get usage:
```
./cppdiscogs
```

List a user's releases by year:
```
./cppdiscogs -l year --user=orenkishon1
```
