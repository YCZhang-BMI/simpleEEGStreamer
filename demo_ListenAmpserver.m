function [ti2] = ListenAmpserver()

command = ['d.exe ','10.10.10.51',' ','9879',' &'];
system(command);
ti2 = tcpip('127.0.0.1',12345,'InputBufferSize',160*4000*10,'Timeout',60,'ByteOrder','littleEndian');
fopen(ti2);
end
