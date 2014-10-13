function pfs_test_shell()
%PFS_TEST_SHELL run several test to check for common problems with pfstools
%matlab interface.
%
% PFS_TEST_SHELL()
%
% The function displays test results and instruction what to do if the
% test has failed. 
%
% Copyright 2009 Rafal Mantiuk

tmp_file = tempname;

cmd = sprintf( '%secho "OK" | cat >''%s''%s', pfs_shell(), tmp_file, pfs_shell( 1 ) );

display( '===========================' );
display( ['Test 1: executing: ' cmd ] );
system( cmd );
display( '  If the lines below show a single line "OK", eveything is correct.' );
display( '  If the lines are empty, the shell most probably cannot be executed.' );
display( '  In such a case check error messages either in matlab command window' );
display( '    (Windows) or in the shell window from which you have started matlab (unix).' );
display( '    Then edit pfs_shell.m to fix these problems.' );
display( '  If the lines contain besides "OK" additional lines of text, fix shell startup' );
display( '    files (/etc/profile, ~/.bash_profile and others) so that no messages are' );
display( '    displayed when shell is started' );
display( '---- START ----' );
display_file( tmp_file );
display( '---- END ----' );

cmd = sprintf( '%swhich pfsin%s', pfs_shell(), pfs_shell( 1 ) );

display( '===========================' );
display( ['Test 2: executing: ' cmd ] );
[status result] = system( cmd );
if( status == 0 )
    display( 'Successful.' );
else
    display( 'Shell failed to find pfstools in the PATH. Make sure that the PATH' );
    display( '  includes directories with pfstools at shell startup.' );
    display( 'Error message: ' );
    display( result );
end

end

function display_file( file_name )

fid=fopen( file_name );
while 1
    tline = fgetl(fid);
    if ~ischar(tline),   break,   end
    disp(tline)
end
fclose(fid);

end