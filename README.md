# torrent
File sharing system with fallback multi-tracker system with synchronization and parallel downloading. Used openssl library for computing hash values of files. Implemented own algorithm for data piece selection in order to download efficiently a file from multiple servers(peers) piece by piece.

Features Implemented:

1.Multi-Tracker(Server) to serve request for all clients.
2.User Authentication
3.File sharing visibility to specific Group and Users
4.Piece Selection Algorithm to balance Load Sharing
5.Compatabile to run on Local Host and different IP and Ports provided by User

