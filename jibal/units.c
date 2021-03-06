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
#include <stdio.h>
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
    units=jibal_units_add(units, 1.0, UNIT_TYPE_DISTANCE, "m");
    units=jibal_units_add(units, 1.0, UNIT_TYPE_ENERGY, "J");
    units=jibal_units_add(units, C_E, UNIT_TYPE_ENERGY, "eV");
    units=jibal_units_add(units, 1.0e-3, UNIT_TYPE_MASS, "g"); /* SI is kg, but we handle prefixes separately */
    units=jibal_units_add(units, C_U, UNIT_TYPE_MASS, "u");
    units=jibal_units_add(units, 1.0, UNIT_TYPE_ANGLE, "rad");
    units=jibal_units_add(units, C_DEG, UNIT_TYPE_ANGLE, "deg");
    units=jibal_units_add(units, C_TFU, UNIT_TYPE_LAYER_THICKNESS, "tfu");
    units=jibal_units_add(units, 1.0, UNIT_TYPE_TIME, "s");
    units=jibal_units_add(units, 1.0, UNIT_TYPE_CHARGE, "C");
    units=jibal_units_add(units, C_E, UNIT_TYPE_CHARGE, "e");
    units=jibal_units_add(units, 1.0, UNIT_TYPE_VOLTAGE, "V");
    units=jibal_units_add(units, 1.0, UNIT_TYPE_CURRENT, "A");
    units=jibal_units_add(units, 1.0, UNIT_TYPE_MAGNETIC_FIELD, "T");
    units=jibal_units_add(units, 1.0e-4, UNIT_TYPE_MAGNETIC_FIELD, "G");
    return units;
}

int jibal_units_count(const jibal_units *units) {
    int n=0;
    while(units) {
        n++;
        units=units->next;
    }
    return n;
}

int jibal_units_print(FILE *out, const jibal_units *units) {
    fprintf(out, " Unit   type        factor\n");
    while(units) {
        fprintf(out, "%5s      %c  %12.7g\n", units->name, units->type, units->f);
        units=units->next;
    }
    return 0;
}

double jibal_units_get(const jibal_units *units, char type, const char *name) {
    if(name == NULL) /* Not allowed */
        return 0.0;
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
                case 'Y':
                    return f*1e22;
                case 'Z':
                    return f*1e21;
                case 'E':
                    return f*1e18;
                case 'P':
                    return f*1e15;
                case 'T':
                    return f*1e12;
                case 'G':
                    return f*1e9;
                case 'M':
                    return f*1e6;
                case 'k':
                    return f*1e3;
                case 'h':
                    return f*1e2;
                /* deca is not supported, since we would need two letters "da" for it */
                case 'd':
                    return f*1e-1;
                case 'c':
                    return f*1e-2;
                case 'm':
                    return f*1e-3;
                case 'u':
                    return f*1e-6;
                case 'n':
                    return f*1e-9;
                case 'p':
                    return f*1e-12;
                case 'f':
                    return f*1e-15;
                case 'a':
                    return f*1e-18;
                case 'z':
                    return f*1e-21;
                case 'y':
                    return f*1e-24;
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
    if(end == value) { /* No conversion, could happen if just a bare unit is given */
        x = 1.0;
    }
    x *= jibal_units_get(units, type, end);
    return x;
}

void jibal_units_free(jibal_units *units) {
    if(!units)
        return;
    jibal_units *this;
    this=units;
    while(this != NULL) {
        jibal_units *next=this->next;
        free(this);
        this=next;
    }
}
