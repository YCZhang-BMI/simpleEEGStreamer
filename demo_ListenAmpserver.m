function [ti2] = ListenAmpserver()
% ti1 = tcpip('10.10.10.51',9877);
% fopen(ti1);
% fprintf(ti1,'(sendCommand cmd_Stop 0 0 0)');
% fprintf(ti1,'(sendCommand cmd_SetSubjectGround 0 0 1)');
% fprintf(ti1,'(sendCommand cmd_Start 0 0 0)'); %AmpÇ©ÇÁÉfÅ[É^Çì«ÇﬂÇÈÇÊÇ§Ç…Ç∑ÇÈ
% fclose(ti1);
% pause(2);
command = ['d.exe ','10.10.10.51',' ','9879',' &'];
system(command);
% pause(2);
ti2 = tcpip('127.0.0.1',12345,'InputBufferSize',160*4000*10,'Timeout',60,'ByteOrder','littleEndian');
fopen(ti2);
end