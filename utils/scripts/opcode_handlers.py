#! /usr/bin/env python
#
# This script generates cross-references to show associated (handled) opcodes
# between the server and client. It will generate files for each client found
# and provide some basic information..such as opcode names and values, server
# handler and whether opcodes are translated on tx/rx.
#
# It's currently limited to the 'Zone' server..but, can be expounded upon to
# include other servers, clients and other criteria and features.


import os
#import pickle

from time import time, ctime


# pipes relativistic information to the console window
VERBOSE = True

# setting this to 'True' will pipe more information than you would ever want!
DEBUG = False

repopath = os.getcwd()[:-14]  # '/utils/scripts'

clients = ['6.2', 'Titanium', 'SoF', 'SoD', 'Underfoot', 'RoF']
servers = ['Login', 'World', 'Zone', 'UCS']

clientops = {}
serverops = {}

encodes = []
decodes = []

handlers = {}
outstreams = {}


def main():
    print('Loading source data...')

    if not loadserveropcodes():
        print('Error! Could not load server opcodes')
        return
    
    if not loadclientopcodes():
        print('Error! Could not load client opcodes')
        return
    
    if not loadclienttranslators():
        print('Error! Could not load client translators')
        return
    
    if not loadserverhandlers():
        print('Error! Could not load server handlers')
        return

    print('Creating output streams...')

    if not createoutputdirectory():
        print('Output directory could not be created...')
        return
        
    if not createoutputstreams():
        print('Error! Could not open output files')
        return

    print('Parsing opcode data...')

    if not parseopcodedata():
        print('Error! Could not parse opcode data')
        return

    print('Destroying output streams...')

    if not destroyoutputstreams():
        print('Error! Could not close output files')
        return


def loadserveropcodes():
    # Server opcodes are auto-enumerated with a starting reference of '1'
    try:
        value = 0

        filename = '{0}{1}{2}'.format(
            repopath,
            '/common',
            '/emu_oplist.h')
        
        if VERBOSE or DEBUG:
            print(filename)

        with open(filename, 'r') as datafile:
            while True:
                dataline = datafile.readline()
                
                if not dataline:
                    break

                vbegin = dataline.find('OP_', 2)
                vend = dataline.find(')', vbegin)
                
                if vbegin == -1:
                    continue
                if vend == -1:
                    continue
                
                value += 1

                if dataline[:1] == 'N':
                    serverops[dataline[vbegin:vend]] = value
                
                    if DEBUG:
                        print('({0}, {1}) = {2}'.format(
                            'Server',
                            dataline[vbegin:vend],
                            serverops[dataline[vbegin:vend]]))

        datafile.close()
        
        filename = '{0}{1}{2}'.format(
            repopath,
            '/common',
            '/mail_oplist.h')
        
        if VERBOSE or DEBUG:
            print(filename)

        with open(filename, 'r') as datafile:
            while True:
                dataline = datafile.readline()
                
                if not dataline:
                    break

                vbegin = dataline.find('OP_', 2)
                vend = dataline.find(')', vbegin)

                if vbegin == -1:
                    continue
                if vend == -1:
                    continue
                
                value += 1
                
                if dataline[:1] == 'N':
                    serverops[dataline[vbegin:vend]] = value
                
                    if DEBUG:
                        print('({0}, {1}) = {2}'.format(
                            'Server',
                            dataline[vbegin:vend],
                            serverops[dataline[vbegin:vend]]))

        datafile.close()
    except:
        return False
    
    return True


def loadclientopcodes():
    badclients = []
    
    for client in clients:
        try:
            filename = '{0}{1}{2}{3}{4}'.format(
                repopath,
                '/utils/patches',
                '/patch_',
                client,
                '.conf')
            
            if VERBOSE or DEBUG:
                print(filename)

            with open(filename, 'r') as datafile:
                while True:
                    dataline = datafile.readline()
                    
                    if not dataline:
                        break

                    kbegin = dataline.find('OP_')
                    kend = dataline.find('=', kbegin)

                    if not kbegin == 0:
                        continue
                    if kend == -1:
                        continue
                    
                    vbegin = dataline.find('0x', kend)
                    vend = vbegin + 6

                    if vbegin == -1:
                        continue

                    value = int(dataline[(vbegin + 2):vend].lower(), 16)

                    if value == 0:
                        continue

                    clientops[client, dataline[kbegin:kend]] = '0x{0}'.format(
                        hex(value)[2:].zfill(4))
                    
                    if DEBUG:
                        print('({0}, {1}) = {2} (int: {3})'.format(
                            client,
                            dataline[kbegin:kend],
                            clientops[client, dataline[kbegin:kend]],
                            value))

            datafile.close()
        except:
            badclients.append(client)

    if len(badclients) > 0:
        badentries = []
        
        for badclient in badclients:
            if VERBOSE or DEBUG:
                print('Deleting client {0} from search criteria...'.format(
                    badclient))
            
            clients.remove(badclient)

            if VERBOSE or DEBUG:
                print('Deleting any partial entries for client {0}...'.format(
                    badclient))
            
            for entry in clientops.keys():
                if entry[0] == badclient:
                    badentries.append(entry)
            
            for badentry in badentries:
                del clientops[badentry]
                
    if len(clients) == 0:
        return False

    return True


def loadclienttranslators():
    for client in clients:
        if client == '6.2':
            shortname = '{0}'.format(
                '/Client62_ops.h')
        else:
            shortname = '{0}{1}{2}'.format(
                '/',
                client,
                '_ops.h')
        
        try:
            filename = '{0}{1}{2}'.format(
                repopath,
                '/common/patches',
                shortname)
            
            if VERBOSE or DEBUG:
                print(filename)

            with open(filename, 'r') as datafile:
                while True:
                    dataline = datafile.readline()
                    
                    if not dataline:
                        break

                    vbegin = dataline.find('OP_', 2)
                    vend = dataline.find(')', vbegin)

                    if vbegin == -1:
                        continue
                    if vend == -1:
                        continue
                    
                    if dataline[:1] == 'E':
                        encodes.append((client, dataline[vbegin:vend]))
                        
                        if DEBUG:
                            print('ENCODE({0}, {1}) [floating index: {2}]'.format(
                                client,
                                dataline[vbegin:vend],
                                encodes.index((client, dataline[vbegin:vend]))))
                    elif dataline[:1] == 'D':
                        decodes.append((client, dataline[vbegin:vend]))
                        
                        if DEBUG:
                            print('DECODE({0}, {1}) [floating index: {2}]'.format(
                                client,
                                dataline[vbegin:vend],
                                decodes.index((client, dataline[vbegin:vend]))))

            datafile.close()
        except:
            # TODO: need to handle
            return False

    # this will need to change if we ever have a client with 100% support
    if len(encodes) == 0 and len(decodes) == 0:
        return False

    return True


def loadserverhandlers():
    # TODO: handle remarked out definitions; add other servers, if possible
    try:
        filename = '{0}{1}{2}'.format(
            repopath,
            '/zone',
            '/client_packet.cpp')
        
        if VERBOSE or DEBUG:
            print(filename)
        
        with open(filename, 'r') as datafile:
            dataline = datafile.readline()
            
            while not dataline[:19] == 'void MapOpcodes() {':
                dataline = datafile.readline()
                
                if not dataline:
                    break
            
            while True:
                dataline = datafile.readline()
                
                if not dataline:
                    break
                if dataline[0:1] == '}':
                    break
                
                kbegin = dataline.find('OP_')
                kend = dataline.find(']', kbegin)
                
                if kbegin == -1:
                    continue
                if kend == -1:
                    continue
                
                vbegin = dataline.find('Client::', kend)
                vend = dataline.find(';', vbegin)
                
                if vbegin == -1:
                    continue
                if vend == -1:
                    continue
                
                handlers['Zone', dataline[kbegin:kend]] = dataline[vbegin:vend]
                
                if DEBUG:
                    print('({0}, {1}) = {2}'.format(
                        'Zone',
                        dataline[kbegin:kend],
                        handlers['Zone', dataline[kbegin:kend]]))

        datafile.close()
    except:
        # should probably do the same for this (badservers)
        # as done for badclients in the above function
        return False 

    return True


def createoutputdirectory():
    outputpath = ''
    
    try:
        outputpath = '{0}{1}'.format(
            repopath,
            '/utils/scripts/opcode_handlers_output')
        
        if VERBOSE or DEBUG:
            print(outputpath)
            
        if not os.path.exists(outputpath):
            os.mkdir(outputpath)
        
        return True
    except:
        return False


def createoutputstreams():
    filename = ''
    
    try:
        for client in clients:
            filename = '{0}{1}{2}{3}'.format(
                repopath,
                '/utils/scripts/opcode_handlers_output/',
                client,
                '_opcode_handlers.txt')
            
            if VERBOSE or DEBUG:
                print(filename)
            
            outstreams[client] = open(filename, 'w')

            message = \
                '------------------------------------------------------\n' \
                '|| Opcode-Handler analysis for \'{0}\' client\n' \
                '|| script-generated file @ {1}\n' \
                '||\n' \
                '|| (only cross-linked (active) opcodes are listed)\n' \
                '------------------------------------------------------\n' \
                '\n'.format(
                    client,
                    ctime(time()))

            outstreams[client].write(message)

            if DEBUG:
                print(message[:-2])
    except:
        for client in clients:
            if client in outstreams.keys():
                outstreams[client].close()
                del outstreams[client]

                if DEBUG:
                    print('->CreatingClientStream(exception): {0}'.format(
                        client))
        
        return False

    return True


def parseopcodedata():
    serveropnames = []
    
    for serveropkey in serverops.keys():
        serveropnames.append(serveropkey)

        if DEBUG:
            print('->ServerOpKey: {0}'.format(
                serveropkey))

    if len(serveropnames) == 0:
        return False
    
    for server in servers:
        # Login, World, UCS not implemented yet
        if not server == 'Zone':
            if DEBUG:
                print('->SkippingServerOpcodeParse: {0}'.format(
                    server))
            
            continue

        handleropnames = []
        
        for handlerkey in handlers.keys():
            if handlerkey[0] == server:
                handleropnames.append(handlerkey[1])
                
                if DEBUG:
                    print('->HandlerKey: {0}'.format(
                        handlerkey[1]))

        if len(handleropnames) == 0:
            return False
        else:
            # sort to process opcode names in ascending order
            handleropnames.sort()

        for client in clients:
            clientopnames = []
            clientencodes = []
            clientdecodes = []

            totalopcodes = 0
            encodedecode = 0
            encodeonly = 0
            decodeonly = 0
            notranslation = 0
            
            for clientopkey in clientops.keys():
                if clientopkey[0] == client:
                    clientopnames.append(clientopkey[1])
                    
                    if DEBUG:
                        print('->ClientOpKey: {0}'.format(
                            clientopkey[1]))

            if len(clientopnames) == 0:
                return False

            for encodeentry in encodes:
                if encodeentry[0] == client:
                    clientencodes.append(encodeentry[1])
                    
                    if DEBUG:
                        print('->EncodeEntry: {0}'.format(
                            encodeentry[1]))

            if len(clientencodes) == 0:
                return False

            for decodeentry in decodes:
                if decodeentry[0] == client:
                    clientdecodes.append(decodeentry[1])
                    
                    if DEBUG:
                        print('->DecodeEntry: {0}'.format(
                            decodeentry[1]))

            if len(clientdecodes) == 0:
                return False
            
            for handleropentry in handleropnames:
                try:
                    clientopindex = clientopnames.index(handleropentry)
                except:
                    clientopindex = -1
                
                if clientopindex > -1:
                    val0 = clientopnames[clientopindex]
                    val1 = serverops[val0]
                    val2 = clientops[(client, val0)]
                    val3 = handlers[(server, val0)]
                    
                    try:
                        val4 = clientencodes.index(val0) > -1
                    except:
                        val4 = False
                    try:
                        val5 = clientdecodes.index(val0) > -1
                    except:
                        val5 = False

                    message = \
                        'Opcode: {0} | {1}: {2} | {3}: {4}\n' \
                        '  {5} Handler: \'{6}\'\n' \
                        '  Encoded: {7} | Decoded: {8}\n\n'.format(
                            val0,
                            server,
                            val1,
                            client,
                            val2,
                            server,
                            val3,
                            val4,
                            val5)

                    outstreams[client].write(message)

                    if DEBUG:
                        print(message[:-2])

                    totalopcodes += 1

                    if val4 and val5:
                        encodedecode += 1
                    elif val4 and not val5:
                        encodeonly += 1
                    elif not val4 and val5:
                        decodeonly += 1
                    elif not val4 and not val5:
                        notranslation += 1
                    
                    if DEBUG:
                        print('->EndOfOpcodeLoop: {0}'.format(
                            val0))

            message = \
                'Statistics -------------------------------------------\n' \
                '|| Server Opcode Type: {0}\n' \
                '|| Handled Opcodes: {1}\n' \
                '|| Bi-directional: {2}\n' \
                '|| Encodes Only: {3}\n' \
                '|| Decodes Only: {4}\n' \
                '|| No Translations: {5}\n' \
                '\n' \
                'Notes ------------------------------------------------\n' \
                '|| Encodes are Server-to-Client and Decodes are' \
                ' Server-from-Client in context\n' \
                '|| \'Bi-directional\' indicates translations are performed on' \
                ' tx/rx packets\n' \
                '|| \'Encodes Only\' indicates translations only on tx packets' \
                ' - does not exclude direct packet rx\n' \
                '|| \'Decodes Only\' indicates translations only on rx packets' \
                ' - does not exclude direct packet tx\n' \
                '|| \'No Translations\' indicates no translations of tx/rx' \
                ' packets\n'.format(
                    server,
                    totalopcodes,
                    encodedecode,
                    encodeonly,
                    decodeonly,
                    notranslation)

            outstreams[client].write(message)

            if DEBUG:
                print(message[:-1])
                print('->EndOfClientLoop: {0}'.format(
                    client))
                    
        if DEBUG:
            print('->EndOfServerLoop: {0}'.format(
                server))

    return True


def destroyoutputstreams():
    for client in clients:
        if client in outstreams.keys():
            outstreams[client].close()
            del outstreams[client]

            if DEBUG:
                print('->DestroyingClientStream: {0}'.format(
                    client))
    
    return True


if __name__ == '__main__':
    main()
