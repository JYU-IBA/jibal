/*
    LibIBA - Library for ion beam analysis
    Copyright (C) 2020 Jaakko Julin <jaakko.julin@jyu.fi>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdlib.h>
#include <string.h>
#include <jibal_units.h>

jibal_units *jibal_units_add(jibal_units *units, double f, char type, char *name) {
    jibal_units *first=units;
    jibal_units *this=malloc(sizeof(jibal_units));
    this->f=f;
    this->type = type;
    this->next = NULL;
    this->name=name;
    if(!first) {
        return this;
    }
    while(units->next) {
        units=units->next;
    }
    units->next=this;
    return first;
}

jibal_units *jibal_units_default() {
    jibal_units *units=NULL;
    units=jibal_units_add(units, C_E, UNIT_TYPE_ENERGY, "eV");
    units=jibal_units_add(units, C_U, UNIT_TYPE_MASS, "u");
    units=jibal_units_add(units, C_DEG, UNIT_TYPE_ANGLE, "deg");
    return units;
}

double jibal_units_get(const jibal_units *units, char type, const char *name) {
    if(*name == '\0') /* Empty. */
        return 1.0;
    while(*name == ' ') {
        name++;
    }
    while(units) {
        if(type && (units->type != type)) {
            units=units->next;
            continue;
        }
        if(strcmp(units->name, name)==0) {
            return units->f; /* Exact match */
        }
        if(strcmp(units->name, name+1)==0) { /* Last letters match, first letter might be a SI prefix */
            double f=units->f;
            switch(*name) { /* First char */
                case 'k':
                    return f*1e3;
                case 'M':
                    return f*1e6;
                case 'G':
                    return f*1e9;
                case 'T':
                    return f*1e12;
                case 'm':
                    return f*1e-3;
                case 'u':
                    return f*1e-6;
                case 'n':
                    return f*1e-9;
                case 'p':
                    return f*1e-12;
                /* No default case since we fall through */
            }
            
        }
        units=units->next;
    }
    return 1.0; /* Actually we should fail maybe with NaN? */
}

double jibal_get_val(const jibal_units *units, char type, const char *value) {
    char *end;
    double x=strtod(value, &end);
    if(end) {
        x *= jibal_units_get(units, type, end);
    }
    return x;
}

void jibal_units_free(jibal_units *units) {
    jibal_units *this;
    this=units;
    while(this != NULL) {
        jibal_units *next=this->next;
        free(this);
        this=next;
    }
}
