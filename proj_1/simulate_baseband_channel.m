%NO_PFILE
function y = simulate_baseband_channel(zf, h, snr, sync_error)
% SIMULATE_BASEBAND_CHANNEL
%
% Simulates a baseband channel given by impulse response h. If SNR is
% specified a white noise signal is added to yield the requested SNR.
% Optionally a negative or positive integer syncronization error can be
% added.
%
% Arguments
%   zf          signal to transmit over the channel h
%   h           impulse resonpse of the baseband channel
%   snr         requested signal to noise ratio.
%               Standard deviation of additive noise at the
%               receiver is given by
%               sigma = sqrt(zupmr'*zupmr/Ntot * 10^(-SNR/10));
%               If SNR set to inf no noise is added.
% sync_error    integer indicating incorrect start of frame.
%   Positive integers -> too late syncronization
%   negative integers -> too early syncronization.
%
% yrec = simulated received signal
%

zf = zf(:);

y = conv(zf,h);
ny = length(y);

if isfinite(snr)
    Py = y'*y/ny; % Power for signal
    sigma = sqrt(Py * 10^(-snr/10)); % Determine correct std for noise based on SNR
    w = sigma*1/sqrt(2)*(randn(ny,1)+1i*randn(ny,1));
    y = y + w;
end

% Adjust start of frame based on sync_error request
if sync_error > 0 
    y = [y(abs(sync_error)+1:end); zeros(abs(sync_error), 1)];
elseif sync_error < 0
    y = [zeros(abs(sync_error), 1); y(1:end-abs(sync_error))];
end