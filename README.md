# QtChat
## Client-Server application
The application allows users to exchange messages and files.
### Client
The client part of the application runs on the user's computer and requests the server to obtain the necessary resources or services. Has a graphical interface.
### Server
The server part of the application runs on a central computer and provides resources and services to client computers. The server includes a database for storing user accounts, messages, and indexing files that have been uploaded to the server. Has no interface and works from the command line.
___
### Already implemented:
+ Сlient side graphical interface
+ Sending messages and files to the server
+ Registration, authorization of users (MD5 + salt for passwords)
+ Database
    + SQLite - storage for text information (messages, path to files on the server)
### In the plans:
+ Improve server multithreading
+ Improve the architecture of server and client code, implement design patterns
+ Organize the creation of custom rooms on the server
+ Add the ability to send a voice message
___
### Instruments
+ Qt 6
+ C++
+ SQLite
___
### Screenshots of the client application
![qtchat](https://github.com/ohmeqwertreysd/QtChat/assets/43862902/446bd2e0-56cc-420c-ad61-830b598485ce)
![qtchat](https://github.com/ohmeqwertreysd/QtChat/assets/43862902/9ba31f20-d673-40e3-aa8d-d88731a03a54)
