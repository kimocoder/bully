/*
    bully - retrieve WPA/WPA2 passphrase from a WPS-enabled AP

    Copyright (C) 2020  kimocoder     <christian@aircrack-ng.org>
    Copyright (C) 2012  Brian Purcell <purcell.briand@gmail.com>

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

#ifndef	_IFACE_H
#define	_IFACE_H

#include "bully.h"

char	BG_CHANS[] = "1,5,9,13,2,6,10,3,7,11,4,8,12";
char	AN_CHANS[] = "36,40,44,48,52,56,58,60";

struct cfreq {
	int	chan;
	int	freq;		/* channel center frequency in MHz */
} freqs[] = {
	{   1, 2412 },
	{   2, 2417 },
	{   3, 2422 },
	{   4, 2427 },
	{   5, 2432 },
	{   6, 2437 },
	{   7, 2442 },
	{   8, 2447 },
	{   9, 2452 },
	{  10, 2457 },
	{  11, 2462 },
	{  12, 2467 },
	{  13, 2472 },
	{  14, 2484 },
	{  34, 5170 },
	{  36, 5180 },
	{  38, 5190 },
	{  40, 5200 },
	{  42, 5210 },
	{  44, 5220 },
	{  46, 5230 },
	{  48, 5240 },
	{  52, 5260 },
	{  56, 5280 },
	{  58, 5290 },
	{  60, 5300 },
	{ 100, 5500 },
	{ 104, 5520 },
	{ 108, 5540 },
	{ 112, 5560 },
	{ 116, 5580 },
	{ 120, 5600 },
	{ 124, 5620 },
	{ 128, 5640 },
	{ 132, 5660 },
	{ 136, 5680 },
	{ 140, 5700 },
	{ 149, 5745 },
	{ 153, 5765 },
	{ 157, 5785 },
	{ 161, 5805 },
	{ 165, 5825 }
#define MAX_CHAN 165
};
#define	NUM_CHAN (sizeof(freqs)/sizeof(struct cfreq))

int set_chan(struct global *G, int chan);
int set_chanx(struct global *G, int chanx);
int next_chan(struct global *G);

#endif /* _IFACE_H */
