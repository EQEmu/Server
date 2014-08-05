#! /usr/bin/env python
# untested on Linux

# This script generates cross-references to show associated (handled) opcodes between the server and client.
# It will generate files for each client found and provide some basic information..such as opcode names and
# values, server handler and whether opcodes are translated on tx/rx.
#
# It's currently limited to the 'Zone' server..but, can be expounded upon to include other servers, clients
# and other criteria and features.


import os
#import pickle

from time import time, ctime


VERBOSE = True # pipes relativistic information to the console window
DEBUG = False # setting this to 'True' will pipe more information to the console window than you would ever want!

repopath = os.getcwd()
repopath = repopath[:-14] # '\utils\scripts' - because I'm lazy and don't want to do it the right way...

clients = [ '6.2', 'Titanium', 'SoF', 'SoD', 'Underfoot', 'RoF' ]
servers = [ 'Login', 'World', 'Zone' ]

clientops = {}
serverops = {}

encodes = []
decodes = []

handlers = {}
outstreams = {}


def main() :
    print('Loading source data...')

    if not LoadServerOpcodes() :
        print('Error! Could not load server opcodes')
        return
    
    if not LoadClientOpcodes() :
        print('Error! Could not load client opcodes')
        return
    
    if not LoadClientTranslators() :
        print('Error! Could not load client translators')
        return
    
    if not LoadServerHandlers() :
        print('Error! Could not load server handlers')
        return

    print('Creating output streams...')

    if not CreateOutputDirectory() :
        print('Output directory already exists or could not be created...')
        #return # existing directory returns a failure..don't exit here..CreateOutputStreams() will catch makdir() problems
        
    if not CreateOutputStreams() :
        print('Error! Could not open output files')
        return

    print('Parsing opcode data...')

    if not ParseOpcodeData() :
        print('Error! Could not parse opcode data')
        return

    print('Destroying output streams...')

    if not DestroyOutputStreams() :
        print('Error! Could not close output files')
        return


def LoadServerOpcodes() :
    # Server opcodes are auto-enumerated with a starting reference of '1'
    try :
        filename = '{0}{1}{2}'.format(repopath, '/common', '/emu_oplist.h')
        
        if VERBOSE or DEBUG : print(filename)
        
        with open(filename, 'r') as datafile :
            value = 0
            serverops['Null'] = value
            
            while True :
                begin = 0
                end = 0
                
                dataline = datafile.readline()
                
                if not dataline : break
                if not dataline[:1] == 'N' : continue
                
                while not dataline[begin:(begin + 1)] == '(' :
                    if begin >= len(dataline) : break
                    else : begin = begin + 1

                end = begin + 1

                while not dataline[(end - 1):end] == ')' :
                    if end >= len(dataline) : break
                    else : end = end + 1
                
                begin = begin + 1 # adjust out the '('
                end = end - 1 # adjust out the ')'
                
                if begin >= end or begin >= len(dataline) : continue
                if end >= len(dataline) : continue
                
                value = value + 1
                
                serverops[dataline[begin:end]] = value
                
                if DEBUG : print('({0}, {1}) = {2}'.format('Server', dataline[begin:end], serverops[dataline[begin:end]]))

        datafile.close()
                
    except : return False
    
    return True


def LoadClientOpcodes() :
    badclients = []
    
    for client in clients :
        try :
            filename = '{0}{1}{2}{3}{4}'.format(repopath, '/utils/patches', '/patch_', client, '.conf')
            
            if VERBOSE or DEBUG : print(filename)

            with open(filename, 'r') as datafile :
                while True :
                    kbegin = 0
                    vbegin = 0
                    kend = 0
                    vend = 0
                    
                    dataline = datafile.readline()
                    
                    if not dataline : break
                    if not dataline[0:3] == 'OP_' : continue

                    kend = kbegin + 3

                    while not dataline[(kend - 1):kend] == '=' :
                        if kend >= len(dataline) : break
                        else : kend = kend + 1

                    kend = kend - 1 # adjust out the '='
                    vbegin = kend + 1
                    vend = vbegin + 6

                    if dataline[vbegin:vend] == '0x0000' : continue
                    
                    clientops[client, dataline[kbegin:kend]] = dataline[vbegin:vend]
                    
                    if DEBUG : print('({0}, {1}) = {2}'.format(client, dataline[kbegin:kend], clientops[client, dataline[kbegin:kend]]))

            datafile.close()

        except : badclients.append(client)

    if len(badclients) > 0 :
        badentries = []
        
        for badclient in badclients :
            if VERBOSE or DEBUG : print('Deleting client {0} from search criteria...'.format(badclient))
            
            clients.remove(badclient)

            if VERBOSE or DEBUG : print('Deleting any partial entries for client {0}...'.format(badclient))
            
            for entry in clientops.keys() :
                if entry[0] == client :
                    badentries.append(entry)
            
            for badentry in badentries :
                del clientops[badentry]
                
    if len(clients) == 0 :
        return False

    return True


def LoadClientTranslators() :
    for client in clients :
        try :
            if client == '6.2' : shortname = '{0}'.format('/Client62_ops.h')
            else : shortname = '{0}{1}{2}'.format('/', client, '_ops.h')
                
            filename = '{0}{1}{2}'.format(repopath, '/common/patches', shortname)
            
            if VERBOSE or DEBUG : print(filename)

            with open(filename, 'r') as datafile :
                while True:
                    begin = 0
                    end = 0
                    
                    dataline = datafile.readline()
                    
                    if not dataline : break
                    if not dataline[:1] == 'E' and not dataline[0:1] == 'D' : continue
                
                    while not dataline[begin:(begin + 1)] == '(' :
                        if begin >= len(dataline) : break
                        else : begin = begin + 1

                    end = begin + 1
                
                    while not dataline[(end - 1):end] == ')' :
                        if end >= len(dataline) : break
                        else : end = end + 1
                
                    begin = begin + 1 # adjust out the '('
                    end = end - 1 # adjust out the ')'
                
                    if begin >= end or begin >= len(dataline) : continue
                    if end >= len(dataline) : continue
                    
                    if dataline[:1] == 'E' :
                        encodes.append((client, dataline[begin:end]))
                        
                        if DEBUG : print('ENCODE({0}, {1}) [floating index: {2}]'.format(client, dataline[begin:end], encodes.index((client, dataline[begin:end]))))
                    elif dataline[:1] == 'D' :
                        decodes.append((client, dataline[begin:end]))
                        
                        if DEBUG : print('DECODE({0}, {1}) [floating index: {2}]'.format(client, dataline[begin:end], decodes.index((client, dataline[begin:end]))))

            datafile.close()

        except : return False # TODO: need to handle

    if len(encodes) == 0 and len(decodes) == 0 :
        return False

    return True


def LoadServerHandlers() :
    # TODO: handle remarked out definitions; add other servers, if possible
    try :
        filename = '{0}{1}{2}'.format(repopath, '/zone', '/client_packet.cpp')
        
        if VERBOSE or DEBUG : print(filename)
        
        with open(filename, 'r') as datafile :
            dataline = datafile.readline()
            
            while not dataline[:19] == 'void MapOpcodes() {' :
                dataline = datafile.readline()
                
                if not dataline : break
            
            while True :
                kbegin = 0
                vbegin = 0
                kend = 0
                vend = 0

                dataline = datafile.readline()
                
                if not dataline : break
                if dataline[0:1] == '}' : break
                
                kbegin = dataline.find('OP_')
                
                if kbegin == -1 : continue
                
                kend = kbegin + 1

                while not dataline[kend:(kend + 1)] == ']' :
                    if kend >= len(dataline) : break
                    else : kend = kend + 1
                
                vbegin = dataline.find('&Client::')
                
                if vbegin == -1 : continue
                
                vbegin = vbegin + 1
                vend = vbegin + 9

                while not dataline[vend:(vend + 1)] == ';' :
                    if vend >= len(dataline) : break
                    else : vend = vend + 1
                
                handlers['Zone', dataline[kbegin:kend]] = dataline[vbegin:vend]
                
                if DEBUG : print('({0}, {1}) = {2}'.format('Zone', dataline[kbegin:kend], handlers['Zone', dataline[kbegin:kend]]))

        datafile.close()

    except : return False # should probably do the same for this (badservers) as done for badclients above

    return True


def CreateOutputDirectory() :
    try :
        outputpath = '{0}{1}'.format(repopath, '/utils/scripts/opcode_handlers_output')
        
        if VERBOSE or DEBUG : print(outputpath)
        
        os.mkdir(outputpath)
        
        return True
    
    except : return False


def CreateOutputStreams() :
    try :
        for client in clients :
            filename = '{0}{1}{2}{3}'.format(repopath, '/utils/scripts/opcode_handlers_output/', client, '_opcode_handlers.txt')
            
            if VERBOSE or DEBUG : print(filename)
            
            outstreams[client] = open(filename, 'w')
            
            outstreams[client].write('******************************************************\n')
            outstreams[client].write('** Opcode-Handler analysis for \'{0}\' client\n'.format(client))
            outstreams[client].write('** script-generated file @ {0}\n'.format(ctime(time())))
            outstreams[client].write('**\n')
            outstreams[client].write('** (only cross-linked (active) opcodes are listed)\n')
            outstreams[client].write('******************************************************\n\n')
    
    except :
        for client in clients :
            if client in outstreams.keys() :
                outstreams[client].close()
                del outstreams[client]
        
        return False

    return True


def ParseOpcodeData() :
    serveropnames = []
    
    for serveropkey in serverops.keys() :
        if serveropkey == 'Null' : continue
        
        if DEBUG : print('->ServerOpKey: {0}'.format(serveropkey))
        
        serveropnames.append(serveropkey)

    if len(serveropnames) == 0 : return False
    
    for server in servers :
        if server == 'Login' or server == 'World' : continue # Login, World not implemented yet

        handleropnames = []
        
        for handlerkey in handlers.keys() :
            if handlerkey[0] == server :
                if DEBUG : print('->HandlerKey: {0}'.format(handlerkey[1]))
                
                handleropnames.append(handlerkey[1])

        if len(handleropnames) == 0 : return False
        else : handleropnames.sort() # sort to process opcode names in ascending order

        for client in clients :
            clientopnames = []
            clientencodes = []
            clientdecodes = []

            notranslation = 0
            encodeonly = 0
            decodeonly = 0
            encodedecode = 0
            totalopcodes = 0
            
            for clientopkey in clientops.keys() :
                if clientopkey[0] == client :
                    if DEBUG : print('->ClientOpKey: {0}'.format(clientopkey[1]))
                    
                    clientopnames.append(clientopkey[1])

            if len(clientopnames) == 0 : return False

            for encodeentry in encodes :
                if encodeentry[0] == client :
                    if DEBUG : print('->EncodeEntry: {0}'.format(encodeentry[1]))
                    
                    clientencodes.append(encodeentry[1])

            if len(clientencodes) == 0 : return False

            for decodeentry in decodes :
                if decodeentry[0] == client :
                    if DEBUG : print('->DecodeEntry: {0}'.format(decodeentry[1]))
                    
                    clientdecodes.append(decodeentry[1])

            if len(clientdecodes) == 0 : return False
            
            for handleropentry in handleropnames :
                try : clientopindex = clientopnames.index(handleropentry)
                except : clientopindex = -1
                
                if clientopindex > -1 :
                    val0 = clientopnames[clientopindex]
                    val1 = serverops[val0]
                    val2 = clientops[(client, val0)]
                    
                    if DEBUG : print('->Opcode: {0} ({1}: {2} | {3}: {4})'.format(val0, server, val1, client, val2))

                    outstreams[client].write('Opcode: {0} | {1}: {2} | {3}: {4}\n'.format(val0, server, val1, client, val2))
                    
                    val3 = handlers[(server, val0)]
                    
                    if DEBUG : print('->{0} Handler: {1}'.format(server, val3))

                    outstreams[client].write('  {0} Handler: {1}\n'.format(server, val3))
                    
                    try : val4 = clientencodes.index(val0) > -1
                    except : val4 = False
                    try : val5 = clientdecodes.index(val0) > -1
                    except : val5 = False
                    
                    if DEBUG : print('Encoded: {0} | Decoded: {1}'.format(val4, val5))

                    outstreams[client].write('  Encoded: {0} | Decoded: {1}\n\n'.format(val4, val5))

                    totalopcodes = totalopcodes + 1

                    if val4 and val5 : encodedecode = encodedecode + 1
                    elif val4 and not val5 : encodeonly = encodeonly + 1
                    elif not val4 and val5 : decodeonly = decodeonly + 1
                    elif not val4 and not val5 : notranslation = notranslation + 1
                    
                    if DEBUG : print('->EndOfOpcodeLoop: {0}'.format(val0))

            if DEBUG : print('->OpcodeCount: {0}'.format(totalopcodes))
            if DEBUG : print('->Translations: (Bi-directional: {0}, EncodeOnly: {1}, DecodeOnly: {2}, NoTranslation: {3})'.format(encodedecode, encodeonly, decodeonly, notranslation))

            outstreams[client].write('Statistics *******************************************\n')
            outstreams[client].write('** Handled Opcodes: {0}\n'.format(totalopcodes))
            outstreams[client].write('** Bi-directional Translations: {0}\n'.format(encodeonly))
            outstreams[client].write('** Encodes Only: {0}\n'.format(encodeonly))
            outstreams[client].write('** Decodes Only: {0}\n'.format(decodeonly))
            outstreams[client].write('** No Translations: {0}\n'.format(notranslation))
            outstreams[client].write('Notes ************************************************\n')
            outstreams[client].write('** \'Bi-directional\' indicates translations are performed on tx/rx packets\n')
            outstreams[client].write('** \'Encodes Only\' indicates translations only on tx packets - does not exclude direct packet rx\n')
            outstreams[client].write('** \'Decodes Only\' indicates translations only on rx packets - does not exclude direct packet tx\n')
            outstreams[client].write('** \'No Translations\' indicates no translations on tx/rx of packets\n')
            
            if DEBUG : print('->EndOfClientLoop: {0}'.format(client))
                    
        if DEBUG : print('->EndOfServerLoop: {0}'.format(server))

    return True


def DestroyOutputStreams() :
    for client in clients :
        if client in outstreams.keys() :
            outstreams[client].close()
            del outstreams[client]
    
    return True


if __name__ == '__main__':
    main()
