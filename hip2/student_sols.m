%NO_PFILE
function [funs, student_id] = student_sols()
%STUDENT_SOLS Contains all student solutions to problems.

% ----------------------------------------
%               STEP 1
% ----------------------------------------
% Set to your birthdate / the birthdate of one member in the group.
% Should a numeric value of format YYYYMMDD, e.g.
% student_id = 19900101;
% This value must be correct in order to generate a valid secret key.
student_id = 20030614;


% ----------------------------------------
%               STEP 2
% ----------------------------------------
% Your task is to implement the following skeleton functions.
% You are free to use any of the utility functions located in the same
% directory as this file as well as any of the standard matlab functions.
    function h = gen_filter()

        % Define the desired frequency response
        f_s = 1;  % Sampling frequency (1 Hz)
        f_pass = 0.05;  % Passband frequency (0.05 Hz)
        f_stop = 0.1;  % Stopband frequency (0.1 Hz)

        % Calculate the filter order (number of coefficients)
        filter_order = 60;

        H_f = [0, f_pass, f_stop, f_s/2] /(f_s/2);         % pi [rad/sample]
        H_a = [1, 1, 0, 0].*H_f*pi*f_s;    % filter amplitude
        h = firpm(filter_order,H_f,H_a,'differentiator');

    end

funs.gen_filter = @gen_filter;


% This file will return a structure with handles to the functions you have
% implemented. You can call them if you wish, for example:
% funs = student_sols();
% some_output = funs.some_function(some_input);

end

