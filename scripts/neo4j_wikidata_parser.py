import json
import codecs
import os
import sys

if len(sys.argv) != 2:
    print("expected wikidata filepath as param")
    exit(1)

WIKIDATA_PATH = sys.argv[1]
# TODO: create folder if it doesn't exists
OUTPUT_PATH   = "output_csv"

def fix_string(s):
    return s.replace("\\", "\\\\").replace("\n", "\\n").replace('"', '""')

def parse_time(time, precision):
    # llega algo como +2013-01-01T00:00:00Z
    state = 0
    year = ''
    month = ''
    day = ''
    for a in time:
        if state == 0:
            if a == '-':
                year += a
            state = 1
        elif state == 1: # leyendo el anio
            if a == '-':
                state = 2
            else:
                year += a
        elif state == 2: # leyendo el mes
            if a == '-':
                state = 3
            else:
                month += a
        elif state == 3: # leyendo el dia
            if a == 'T':
                break
            else:
                day += a

    if int(month) == 0 or precision < 10:
        fix_month = ''
    else:
        fix_month = str(int(month))

    if int(day) == 0 or precision < 11:
        fix_day = ''
    else:
        fix_day = str(int(day))


    return year, fix_month, fix_day

def xstr(s):
    if s is None:
        return ''
    return str(s)

entity_type  = ""
relationship = ""
item_to      = ""
string_value = ""

# wikidata json
json_file         = codecs.open(WIKIDATA_PATH, 'r', encoding='utf-8')

# nodes
csv_entities      = codecs.open(OUTPUT_PATH + '/entity.csv',      'w', encoding='utf-8')
csv_strings       = codecs.open(OUTPUT_PATH + '/string.csv',      'w', encoding='utf-8')
csv_time          = codecs.open(OUTPUT_PATH + '/time.csv',        'w', encoding='utf-8')
csv_quantity      = codecs.open(OUTPUT_PATH + '/quantity.csv',    'w', encoding='utf-8')
csv_url           = codecs.open(OUTPUT_PATH + '/url.csv',         'w', encoding='utf-8')
csv_monolingual   = codecs.open(OUTPUT_PATH + '/monolingual.csv', 'w', encoding='utf-8')
csv_commons       = codecs.open(OUTPUT_PATH + '/commons.csv',     'w', encoding='utf-8')
csv_globe         = codecs.open(OUTPUT_PATH + '/globe.csv',       'w', encoding='utf-8')
csv_claims        = codecs.open(OUTPUT_PATH + '/claims.csv',      'w', encoding='utf-8')

# relationships
csv_claims_rel     = codecs.open(OUTPUT_PATH + '/csv_claims_rel.csv',     'w', encoding='utf-8')
csv_qualifiers_rel = codecs.open(OUTPUT_PATH + '/csv_qualifiers_rel.csv', 'w', encoding='utf-8')

csv_claims.write        ('id:ID,:LABEL\n')
csv_entities.write      ('id:ID,label,description,:LABEL\n')
csv_commons.write       ('id:ID,value,:LABEL\n')
csv_url.write           ('id:ID,value,:LABEL\n')
csv_strings.write       ('id:ID,value,:LABEL\n')
csv_time.write          ('id:ID,value,timezone,before,after,precision,year:long,month:int,day:int,:LABEL\n')
csv_quantity.write      ('id:ID,value,unit,upperBound,lowerBound,:LABEL\n')
csv_globe.write         ('id:ID,value,latitude,longitude,altitude,precision,globe,:LABEL\n')
csv_monolingual.write   ('id:ID,value,language,:LABEL\n')

csv_claims_rel.write    (':START_ID,:END_ID,property,claim,:TYPE\n')
csv_qualifiers_rel.write(':START_ID,:END_ID,property,:TYPE\n')

# first line is a {
line = json_file.readline()
line_number = 1

#initializing generated ids
url_generated_id       = 0
mono_generated_id      = 0
time_generated_id      = 0
claim_generated_id     = 0
globe_generated_id     = 0
string_generated_id    = 0
commons_generated_id   = 0
quantity_generated_id  = 0
qualifier_generated_id = 0
reference_generated_id = 0

unknown_datatypes = []
unknown_entitytypes = []

while True:
    line = json_file.readline()
    line_number += 1
    if not line:
        break

    # last line in json is different
    if line[-2] == ',':
        fixline = line[:-2]
    else:
        fixline = line[:-1]

    try:
        j = json.loads(fixline)
    except ValueError:
        print('invalid line in json file: ' + str(line_number) + '.')
        continue

    entity_id   = j['id']
    entity_type = j['type']

    label_en = ""
    description_en = ""
    if 'labels' in j and 'en' in j['labels']:
        label_en = j['labels']['en']['value']

    if 'descriptions' in j and 'en' in j['descriptions']:
        description_en = j['descriptions']['en']['value']

    # write the item/property in entities.csv
    if entity_type == 'item':
        csv_entities.write('{},"{}","{}",Item\n'.format(entity_id, fix_string(label_en), fix_string(description_en) ))
    elif entity_type == 'property':
        csv_entities.write('{},"{}","{}",Property\n'.format(entity_id, fix_string(label_en), fix_string(description_en) ))
    else:
        print('Entity its not Item nor Property, at line:' + str(line_number))

    # TODO: remember unknown datatypes and print at end

    if 'claims' in j:
        claims = j['claims']
        for c in claims:
            relationship = c
            p = j['claims'][c]
            for p2 in p:
                claim_generated_id += 1
                claim = 'CL' + str(claim_generated_id)
                if 'qualifiers' in p2:
                    qualifiers = p2['qualifiers']
                    for c in qualifiers:
                        q = qualifiers[c]
                        for q2 in q:
                            qualifier_generated_id += 1

                            if 'datavalue' not in q2:
                                continue

                            datatype = q2['datavalue']['type']

                            if datatype == 'wikibase-entityid':
                                entity_type = q2['datatype']
                                entity_number = q2['datavalue']['value']['numeric-id']

                                if entity_type == 'wikibase-item':
                                    entity = f'Q{entity_number}'
                                elif entity_type == 'wikibase-property':
                                    entity = f'P{entity_number}'
                                else:
                                    if entity_type not in unknown_entitytypes:
                                        print("unknown entity_type:", entity_type)
                                        unknown_entitytypes.append(entity_type)
                                    entity = f'UNK{entity_number}'
                                csv_qualifiers_rel.write("{},{},{},QUALIFIER\n".format(claim, entity, c))

                            elif datatype == 'time':
                                time_generated_id += 1
                                time      = q2['datavalue']['value']['time']
                                timezone  = str(q2['datavalue']['value']['timezone'])
                                before    = str(q2['datavalue']['value']['before'])
                                after     = str(q2['datavalue']['value']['after'])
                                precision = str(q2['datavalue']['value']['precision'])
                                year, month, day = parse_time(time, int(precision))
                                csv_time.write("T{},{},{},{},{},{},{},{},{},Time\n".format( \
                                    time_generated_id, \
                                    time,              \
                                    timezone,          \
                                    before,            \
                                    after,             \
                                    precision,         \
                                    year,              \
                                    month,             \
                                    day ))
                                csv_qualifiers_rel.write("{},T{},{},QUALIFIER\n".format(claim, time_generated_id, c))

                            elif datatype == 'string':
                                string_generated_id += 1
                                string_value = q2['datavalue']['value']
                                csv_strings.write('S'+str(string_generated_id) + ',"'+ fix_string(string_value) + ',String"\n')
                                csv_qualifiers_rel.write("{},S{},{},QUALIFIER\n".format(claim, str(string_generated_id), c))

                            elif datatype == 'quantity':
                                quantity_generated_id += 1
                                amount     = q2['datavalue']['value']['amount']
                                unit       = q2['datavalue']['value']['unit']
                                upperBound = q2['datavalue']['value']['upperBound']
                                lowerBound = q2['datavalue']['value']['lowerBound']
                                csv_quantity.write('QT'+str(quantity_generated_id)+','+amount+','+unit+','+upperBound+','+lowerBound+',Quantity\n')
                                csv_qualifiers_rel.write("{},QT{},{},QUALIFIER\n".format(claim, str(quantity_generated_id), c))

                            elif datatype == 'url':
                                url_generated_id += 1
                                url_value = fix_string(q2['datavalue']['value'])
                                csv_url.write('U' + str(url_generated_id) + ',"' + url_value + '",Url\n')
                                csv_qualifiers_rel.write("{},U{},{},QUALIFIER\n".format(claim, str(url_generated_id), c))

                            elif datatype == 'monolingualtext':
                                mono_generated_id += 1
                                mono_text = fix_string(q2['datavalue']['value']['text'])
                                mono_lang = q2['datavalue']['value']['language']
                                csv_monolingual.write('MT'+ str(mono_generated_id) + ',"' + mono_text + '",' + mono_lang + ',Monotext\n')
                                csv_qualifiers_rel.write("{},MT{},{},QUALIFIER\n".format(claim, str(mono_generated_id), c))

                            elif datatype == 'commonsMedia':
                                commons_generated_id += 1
                                commons_value = fix_string(q2['datavalue']['value'])
                                csv_commons.write('CM' + str(commons_generated_id) + ',"' + commons_value + '",Commonsmedia\n')
                                csv_qualifiers_rel.write("{},CM{},{},QUALIFIER\n".format(claim, str(commons_generated_id), c))

                            elif datatype == 'globecoordinate':
                                globe_generated_id += 1
                                latitude    = xstr(q2['datavalue']['value']['latitude'])
                                longitude   = xstr(q2['datavalue']['value']['longitude'])
                                altitude    = xstr(q2['datavalue']['value']['altitude'])
                                precision   = xstr(q2['datavalue']['value']['precision'])
                                globe       = xstr(q2['datavalue']['value']['globe'])
                                globe_value = 'lat:' + latitude + ' lon:' + longitude
                                csv_globe.write('GC'+str(globe_generated_id)+','+globe_value+','+latitude+','+longitude+','+altitude+','+precision+','+globe+',GlobeCoordinate\n')
                                csv_qualifiers_rel.write("{},GC{},{},QUALIFIER\n".format(claim, str(globe_generated_id), c))
                            else:
                                if datatype not in unknown_datatypes:
                                    print("unknown datatype:", datatype)
                                    unknown_datatypes.append(datatype)

                if 'datatype' in p2['mainsnak']:
                    if 'datavalue' not in p2['mainsnak']:
                            continue
                    datatype = p2['mainsnak']['datavalue']['type']
                    csv_claims.write(claim + ',CLAIM\n')

                    if datatype == 'wikibase-entityid':
                        entity_type = p2['mainsnak']['datatype']
                        entity_number = p2['mainsnak']['datavalue']['value']['numeric-id']

                        if entity_type == 'wikibase-item':
                            entity = f'Q{entity_number}'
                        elif entity_type == 'wikibase-property':
                            entity = f'P{entity_number}'
                        else:
                            if entity_type not in unknown_entitytypes:
                                print("unknown entity_type:", entity_type)
                                unknown_entitytypes.append(entity_type)
                            entity = f'UNK{entity_number}'
                        csv_claims_rel.write("{},{},{},{},CLAIM\n".format(entity_id, entity, relationship, claim))

                    elif datatype == 'string':
                        string_generated_id += 1
                        string_value = p2['mainsnak']['datavalue']['value']
                        csv_strings.write('S'+str(string_generated_id) + ',"'+ fix_string(string_value) + ',String"\n')
                        csv_claims_rel.write("{},S{},{},{},CLAIM\n".format(entity_id, str(string_generated_id), relationship, claim))

                    elif datatype == 'time':
                        time_generated_id += 1
                        time      = p2['mainsnak']['datavalue']['value']['time']
                        timezone  = str(p2['mainsnak']['datavalue']['value']['timezone'])
                        before    = str(p2['mainsnak']['datavalue']['value']['before'])
                        after     = str(p2['mainsnak']['datavalue']['value']['after'])
                        precision = str(p2['mainsnak']['datavalue']['value']['precision'])
                        year, month, day = parse_time(time, int(precision))
                        csv_time.write('T'+str(time_generated_id) +','+time+','+timezone+','+before+','+after+','+precision+','+year+','+month+','+day+',Time\n')
                        csv_claims_rel.write("{},T{},{},{},CLAIM\n".format(entity_id, str(time_generated_id), relationship, claim))

                    elif datatype == 'quantity':
                        quantity_generated_id += 1
                        amount      = p2['mainsnak']['datavalue']['value']['amount']
                        unit        = p2['mainsnak']['datavalue']['value']['unit']
                        upperBound  = p2['mainsnak']['datavalue']['value']['upperBound']
                        lowerBound  = p2['mainsnak']['datavalue']['value']['lowerBound']
                        csv_quantity.write('QT'+str(quantity_generated_id)+','+amount+','+unit+','+upperBound+','+lowerBound+',Quantity\n')
                        csv_claims_rel.write("{},QT{},{},{},CLAIM\n".format(entity_id, str(quantity_generated_id), relationship, claim))

                    elif datatype == 'url':
                        url_generated_id += 1
                        url_value = fix_string(p2['mainsnak']['datavalue']['value'])
                        csv_url.write('U' + str(url_generated_id) + ',"' + url_value + '"\n')
                        csv_claims_rel.write("{},U{},{},{},CLAIM\n".format(entity_id, str(url_generated_id), relationship, claim))

                    elif datatype == 'monolingualtext':
                        mono_generated_id += 1
                        mono_text = fix_string(p2['mainsnak']['datavalue']['value']['text'])
                        mono_lang = p2['mainsnak']['datavalue']['value']['language']
                        csv_monolingual.write('MT'+ str(mono_generated_id) + ',"' + mono_text + '",' + mono_lang + ',Monotext\n')
                        csv_claims_rel.write("{},MT{},{},{},CLAIM\n".format(entity_id, str(mono_generated_id), relationship, claim))

                    elif datatype == 'commonsMedia':
                        commons_generated_id += 1
                        commons_value = fix_string(p2['mainsnak']['datavalue']['value'])
                        csv_commons.write('CM' + str(commons_generated_id) + ',"' + commons_value + '",Commonsmedia\n')
                        csv_claims_rel.write("{},CM{},{},{},CLAIM\n".format(entity_id, str(commons_generated_id), relationship, claim))

                    elif datatype == 'globecoordinate':
                        globe_generated_id += 1
                        latitude    = xstr(p2['mainsnak']['datavalue']['value']['latitude'])
                        longitude   = xstr(p2['mainsnak']['datavalue']['value']['longitude'])
                        altitude    = xstr(p2['mainsnak']['datavalue']['value']['altitude'])
                        precision   = xstr(p2['mainsnak']['datavalue']['value']['precision'])
                        globe       = xstr(p2['mainsnak']['datavalue']['value']['globe'])
                        globe_value = 'lat:' + latitude + ' lon:' + longitude
                        csv_globe.write('GC'+str(globe_generated_id)+','+globe_value+','+latitude+','+longitude+','+altitude+','+precision+','+globe+',Globecoordinate\n')
                        csv_claims_rel.write("{},GC{},{},{},CLAIM\n".format(entity_id, str(globe_generated_id), relationship, claim))
                    else:
                        if datatype not in unknown_datatypes:
                            print("unknown datatype:", datatype)
                            unknown_datatypes.append(datatype)


# close all open files
json_file.close()
csv_url.close()
csv_time.close()
csv_globe.close()
csv_claims.close()
csv_commons.close()
csv_strings.close()
csv_quantity.close()
csv_entities.close()
csv_monolingual.close()
csv_claims_rel.close()
csv_qualifiers_rel.close()
