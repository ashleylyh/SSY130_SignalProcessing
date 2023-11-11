%NO_PFILE
function str = bits2string(bits)
% BITS2STRING Converts a sequence of bits to an ASCII string. Assumes input
% is a series of zero/nonzero numeric values, encoded with
% most-significant-bit first.
% Usage example:
% bits = [0 1 1 0 1 0 0 0 0 1 1 0 0 1 0 1 0 1 1 0 1 1 0 0 0 1 1 0 1 ...
% 1 0 0 0 1 1 0 1 1 1 1 0 0 1 0 0 0 0 0 0 1 1 1 0 1 1 1 0 1 1 0 1 1 ...
% 1 1 0 1 1 1 0 0 1 0 0 1 1 0 1 1 0 0 0 1 1 0 0 1 0 0 0 0 1 0 0 0 0 ...
% 1];
% disp(bits2string(bits));

bits = logical(bits(:));
Nbits = length(bits);
if rem(Nbits,8)>0
    error('The number of bits must be a multiple of 8');
end

% Group bits into bytes
bytes = reshape(char('0' + bits),8,[]).';

% Convert bytes to ASCII characters
chars = char(bin2dec(bytes));

% Convert ASCII characters to row vector of characters
str = chars(:).';