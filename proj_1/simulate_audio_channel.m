%NO_PFILE
function [y, idx_start, h] = simulate_audio_channel(x, f_s, snr, sync_err)
%SIMULATE_AUDIO_CHANNEL 
% 
% Simulates tha audio channel (loudspeaker, air, microphone) with additive
% white noise and a random delay respresenting the synchronization error
% between receiver and transmitter.
%
% Arguments:
%   x           Signal to transmit over channel
%   f_s         Sample rate of signal [Hz]
%   snr         Requested signal to noise ratio [dB]
%               Standard deviation of additive noise at the receiver is
%               given by sigma = sqrt(zupmr'*zupmr/Ntot * 10^(-SNR/10));
%               Set to inf to disable noise source.
%   sync_err    Synchronization error [samples at fs]
% 
% Returns:
%   y           Simulated received signal
%   idx_start   Index of y that corresponds to start of signal, excluding
%               any sync error.
%   h           Simulated channel impulse response


if ~isreal(x)
    error('Input signal cannot be complex');
end

%Ensure input is a column vector
x = x(:);

N = length(x);

y_time = 5; %Total length of output vector [seconds]

N_y = round(y_time * f_s);

% Preallocate result vector, make fairly long, insert input signal in
% middle of vector
idx_start = round(N_y/2) + 1;
y = [zeros(idx_start, 1); x; zeros(N_y - idx_start, 1)];

% Generate simulated channel model. Will use a simple IIR representation of
% form H = B(z)/A(z).

% Approximate channel as bandpass nature, use type 1 chebychev filter to
% give some moderate passband ripple and phase variation
ch_n = 6;   % Channel order
ch_r = 4;   % Channel passband ripple [dB]
ch_b = [50, 6000]*2/f_s;
[b, a] = cheby1(ch_n, ch_r, ch_b);

% Generate channel impulse response
h = impz(b, a);

%freqz(h); %Execute to display the simulated channel gain/phase response

y = conv(h, y);

% Apply sync error
if sync_err < 0
    y = y(abs(sync_err):end); 
else
    y = y(1:end-abs(sync_err)); 
end

% Optionally add white noise
if isfinite(snr)
    Py = y'*y/N; % Only count Power for active signal
    sigma = sqrt(Py * 10^(-snr/10));
    y = y + sigma*randn(length(y),1);
end

end

