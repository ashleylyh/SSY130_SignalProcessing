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


    function [Xfilt,Pplus] = kalm_filt(Y,A,C,Q,R,x0,P0)
        
        % [Xfilt,Pplus] = kalmfilt(Y,A,C,Q,R,x0,P0)
        % Matlab function for Kalman filtering
        % Inputs:
        %  Y    Measured signal, matrix of size [p,N] where N is the number
        %       of samples and p the number of outputs
        %  A    System dynamics matrix, size [n,n]
        %  C    Measurement matrix, size [p,n]
        %  Q    Covariance matrix of process noise, size [n,n]
        %  R    Covariance matrix of measurement noise, size [p,p]
        %  x0   Estimate of x(0), size [n,1]. Defaults to a zero vector if
        %       not supplied.
        %  P0   Error covariance for x(0), size [n,n]. Defaults to the
        %       identity matrix if not supplied.
        % Outputs:
        % Xfilt Kalman-filtered estimate of the state, size [n,N]
        % Pplus Covariance matrix for last sample, size [n,n]
        
        [p,N] = size(Y);        % N = number of samples, p = number of "sensors"
        n = length(A);          % n = system order
        Xpred = zeros(n,N+1);   % Kalman predicted states
        Xfilt = zeros(n,N);     % Kalman filtered states (after using the meeasurment)
        
        if nargin < 7
            P0=eye(n);          % Default initial covariance
        end
        if nargin < 6
            x0=zeros(n,1);      % Default initial states
        end
        
        % Filter initialization:
        Xpred(:,1) = x0;        % Index 1 means time 0
        P = P0;                 % Initial covariance matrix (uncertainty)
        
        % Kalman filter iterations:
        for t=1:N
            % Filter update based on measurement
            % Xfilt(:,t) = Xpred(:,t) + ...
            Xfilt(:,t) = 0; %TODO: This line is missing some code!
            
            % Uncertainty update
            Pplus = 0; %TODO: This line is missing some code!
            
            % Prediction
            Xpred(:,t+1) = 0; %TODO: This line is missing some code!
            
            % Uncertainty propagation
            P = 0; %TODO: This line is missing some code!
        end
    end

funs.kalm_filt = @kalm_filt;


% This file will return a structure with handles to the functions you have
% implemented. You can call them if you wish, for example:
% funs = student_sols();
% some_output = funs.some_function(some_input);

end

