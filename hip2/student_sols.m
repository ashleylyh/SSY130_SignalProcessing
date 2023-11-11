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
student_id = 0;


% ----------------------------------------
%               STEP 2
% ----------------------------------------
% Your task is to implement the following skeleton functions.
% You are free to use any of the utility functions located in the same
% directory as this file as well as any of the standard matlab functions.


    function h = gen_filter()
        h = 0; %TODO: This line is missing some code!
    end

funs.gen_filter = @gen_filter;


% This file will return a structure with handles to the functions you have
% implemented. You can call them if you wish, for example:
% funs = student_sols();
% some_output = funs.some_function(some_input);

end

