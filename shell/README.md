Commands from CP/M for eZ80:
    CP/M group:
        dir     directory listing: DIR [<disk>][<name>.<ext>]
        era     file erase: ERA [<disk>]<name>.<ext>
        ren     file rename: REN [<disk>]<name>.<ext>=[<disk>]<name>.<ext>
        type    file typing on console: TYPE [<disk>]<name>.<ext>
        save    TPA block save: SAVE <block_cnt> [<disk>]<name>.<ext>
        user    user selection: USER <user_num>
        x:      disk select
        xxx     run external command xxx.COM

    System group:
        help    list all internal command
        time    set/show current time
        date    set/show current date
        reset   reset CP/M (jump to reset vector)
        xdir    list all FAT16/32 directory entries
        xtype   print selected file on terminal
        cd      change directory (for selected disk only)
        tpa     run TPA program: TPA [<disk>]<name>.<ext>
        eject   unmount card from system and allow removing it
        mbr     show MBR sector
        mount   mount card to system
        info    print SD/MMC card info

Commands from MOS for Agon Light:

        DIR or CAT or .
        LOAD
        SAVE
        DELETE or ERASE
        JMP
        RUN
        CD
        RENAME
        MKDIR
        SET

Commands from Windows cmd.exe:

        ASSOC           Show or modify file extension associations
        ATTRIB          Show or change DOS file attributes
        CALL            Invoke a batch file from inside another
        CD (CHDIR)      Change current default directory
        CHOICE          Wait for an keypress from a selectable list
        CLS             Clear the console screen
        COPY            Copy file
        CTTY            Change input/output device
        DATE            Show or change the system date
        DEL (ERASE)     Delete a file or set of files
        DIR             List the contents of a directory
        ECHO            Copy text directly to the console output
        ENDLOCAL        End localization of environment changes in a batch file
        FTYPE           Show or modify open commands associated with file types
        HELP            Show brief help details on a topic
        MD (MKDIR)      Create a subdirectory
        MKLINK  Create a symbolic link
        MORE            Display output in pages
        MOVE            Move a file, set of files or directory tree
        PATH            Set or show the search path
        PAUSE           Suspend execution of a batch file
        POPD            Restore the directory to the last one saved with PUSHD
        PROMPT          Change the command prompt
        PUSHD           Change to a new directory, saving the current one
        REN (RENAME)    Rename a file
        RD (RMDIR)      Delete a subdirectory
        SET             Set or show environment variables
        SETLOCAL        Start localization of environment changes in a batch file
        START           Start a program, or open a document in the associated program
        TIME            Set or show the current system time
        TITLE           Set the window title for the CMD session
        TYPE            Type the contents of a text file
        VER             Show the current version of CMD
        VOL             Show the volume label of a disk device
        XCOPY           Copy source files or directory trees to a destination
        EXIT            Close down CMD

Commands from ReactOS cmd.exe:

        ?
        activate
        alias
        assoc
        beep
        call		 BATCHONLY
        cd		 SPECIAL
        chdir		 SPECIAL
        choice
        cls
        color
        copy
        ctty
        date
        del
        delete
        delay
        dir		 SPECIAL
        dirs
        echo		 SPECIAL
        echos
        echoerr		 SPECIAL
        echoserr
        endlocal
        erase
        exit
        for
        free
        goto		 BATCHONLY
        history
        if
        memory
        md		 SPECIAL
        mkdir		 SPECIAL
        mklink
        move
        msgbox
        path
        pause
        popd
        prompt
        pushd
        rd		 SPECIAL
        rmdir		 SPECIAL
        rem
        ren
        rename
        replace
        screen
        set
        setlocal
        shift		 BATCHONLY
        start
        time
        timer
        title
        type
        ver
        verify
        vol
        window
