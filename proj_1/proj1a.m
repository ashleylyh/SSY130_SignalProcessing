%NO_PFILE
% Sim OFDM_1A
% Evaluate the performance of the OFDM communication scheme, part A.
%

% Perform the following steps:
%   1) In student_sols.m, update the student_id variable as described
%   there.
%
%   2) In student_sols.m, complete all the partially-complete functions.
%   (Indicated by a '%TODO: ...' comment). Note that all the functions you
%   need to complete are located in student_sols.m (and only in
%   student_sols.m). You can test these functions by running this file,
%   which will apply a self-test to your functions. When all functions pass
%   the self-test, a unique password will be printed in the terminal. Be
%   sure to include this password in your submission.
%
%   3) Now that the functions in student_sols.m are completed, continue
%   working with this file. Notably, your finished functions will be used
%   to evaluate the behavior of the assignment.
%
% -------------------------------------------------------------------------
%                    Note on function handles
% -------------------------------------------------------------------------
% In this file, we will make use of function handles. A function handle is
% a variable that refers to a function. For example:
%
% x = @plot
%
% assigns a handle to the plot function to the variable x. This allows to
% for example do something like
%
% x(sin(linspace(0,2*pi)))
%
% to call the plot function. Usefully for you, there exist function handles
% to all the functions you've written in student_sols.m. See below for
% exactly how to call them in this assignment.
%
% -------------------------------------------------------------------------
%                    Final notes
% -------------------------------------------------------------------------
%
% The apply_tests() function will set the random-number generator to a
% fixed seed (based on the student_id parameter). This means that repeated
% calls to functions that use randomness will return identical values. This
% is in fact a "good thing" as it means your code is repeatable. If you
% want to perform multiple tests you will need to call your functions
% several times after the apply_tests() function rather than re-running
% this entire file.
%
% Note on debugging: if you wish to debug your solution (e.g. using a
% breakpoint in student_sols.m), comment out the line where the apply_tests
% function is called in the hand-in/project script. If you do not do this
% then you'll end up debugging your function when it is called during the
% self-test routine, which is probably not what you want. (Among other
% things, you won't be able to control the input to your functions).
%
% Files with a .p extension are intentionally obfusticated (they cannot
% easily be read). These files contain the solutions to the tasks you are
% to solve (and are used in order to self-test your code). Though it is
% theoretically possible to break into them and extract the solutions,
% doing this will take you *much* longer than just solving the posed tasks
% =)

% Do some cleanup
clc
clear variables
format short eng

% Perform all self-tests of functions in student_sol.m
apply_tests();

% Load student-written functions
funs = student_sols();

% ----------------------------------------------------------------------
%                           NOTE!
% ----------------------------------------------------------------------
% You can call your functions at any time using the funs structure. For
% example, to add a cyclic prefix of length N_cp to some vector x:
% x_cp = funs.add_cyclic_prefix(x, N_cp);
%

% Here, we will set up the simulation parameters. You will need to change
% these parameters to evaluate the system behavior as described in the
% project report.

N = 272;         % Number of OFDM (QPSK) symbols to transmit.   
N_cp = 0;        % Length of cyclic prefix
snr = inf;       % Receiver side SNR [dB]
sync_err = 0;    % Negative values imply early frame sync
channel_known = true;   %Set true to use the known channel, false to use the unknown channel

% Text to send, must correspond to at least N OFDM symbols
tx_str = ['Alice: Would you tell me, please, which way I ought to go from here? ' ...
    'The Cheshire Cat: That depends a good deal on where you want to get to. ' ...
    'Alice: I don''t much care where. ' ...
    'The Cheshire Cat: Then it doesn''t much matter which way you go. ' ...
    'Alice: ...So long as I get somewhere. ' ...
    'The Cheshire Cat: Oh, you''re sure to do that, if only you walk long enough'];

pilot_str = ['Mad Hatter: "Why is a raven like a writing-desk?"' ...
    '"Have you guessed the riddle yet?" the Hatter said, turning to Alice again.' ...
    '"No, I give it up," Alice replied: "What''s the answer?"' ...
    '"I haven''t the slightest idea," said the Hatter'];

% Clip to right length. An ASCII character is 8 bits in length, while a
% QPSK symbol encodes 2 bits, so we will send N/4 ASCII characters.
tx_str = tx_str(1:N/4);
pilot_str = pilot_str(1:N/4);

% Convert the string to bits
tx = string2bits(tx_str);
pilot = string2bits(pilot_str);

% Define a baseband channel

%h = zeros(60,1); h(1) = 1;   % Ideal
%h = zeros(60,1); h(1) = 0.5; % Ideal, scaled magnitude
%h = zeros(60,1); h(1) = exp(1j*1/2);    % Ideal, phase shift by 1/2 radian (~28 degrees)
h = 0.8.^(0:59)';            % LP model
%h = zeros(60,1); h(1) = 0.5; h(9) = 0.5; % Multipath (2 paths)
%h = randn(60,1);             % Random Gaussian 

% Plot the channel response
figure(1);
subplot(2,1,1);
title('Channel response');
plot(abs(fft(h, N)));
xlabel('k');
ylabel('|H(k)|');
subplot(2,1,2);
plot(angle(fft(h, N)));
xlabel('k');
ylabel('arg(H(k))');


% Utility function to remove non-printable characters from a string
clean_str = @(str) regexprep(str, '[^ -~]+', '_');

% Simulate baseband OFDM communication

if channel_known
   [rx, evm, ber, symbs] = funs.sim_ofdm_known_channel(tx, h, N_cp, snr, sync_err); 
else
    tx_s.d = tx;
    tx_s.p = pilot;
    [rx, evm, ber, symbs] = funs.sim_ofdm_unknown_channel(tx_s, h, N_cp, snr, sync_err);
end

if length(rx) <= 1
    warning('Implement sim_ofdm_known_channel/sim_ofdm_unknown_channel!');
else
    % Convert the recieved bits to a string, replacing non-printable characters
    % with an underscore
    rx_str = clean_str(bits2string(rx));

    fprintf('Transmitted: ''%s''\nRecieved:    ''%s''\n', tx_str, rx_str);
    fprintf('EVM: %.3g, BER: %.3g\n', evm, ber);

    % Draw a constellation plot of the recieved symbols, pre- and post-equalization
    % Recieved symbols are drawn in varying sizes so symbols in repeated
    % locations will be visible
    figure(2);
    plot_constallation(symbs.tx, symbs.rx_pe);
    title('Pre-equalization symbol constellation');

    figure(3);
    plot_constallation(symbs.tx, symbs.rx_e);
    title('Post-equalization symbol constellation');
end