/* -*- tab-width: 4; -*-
 * Copyright 2010 Nathan Gelderloos, Ethan Zonca, Nathan Phillip Brink
 *
 * This file is part of SlatePermutate.
 *
 * SlatePermutate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SlatePermutate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with SlatePermutate.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Assumes that WebAdvisor_scripts.js for WebAdvisor-2.x is loaded,
 * displayFormHTML() or something was called and thus
 * readURLParameters() was called. We attempt to extract TOKENIDX and
 * update the URL GET parameter to contain TOKENIDX so that, after the
 * user is logged in, the webadvisor.php will know a valid TOKENIDX to
 * be used to forge a form for the user. We currently assume we're on
 * a login form too.
 */

(function() {
	var slate_permutate_input_login;

		var slate_permutate_onload = function() {

				/*
				 * Override the login form's submission handler to catch the
				 * case where we're still trying to load the TOKENIDX or
				 * something else.
				 */
				var inputs = document.getElementsByTagName('input');
				for (var i = 0; i < inputs.length; i ++)
				{
						slate_permutate_input_login = inputs.item(i);
						if (slate_permutate_input_login.getAttribute('name') == 'SUBMIT2')
								break;
				}
				slate_permutate_input_login.setAttribute('value', 'Discovering TOKENIDX...');
				slate_permutate_input_login.setAttribute('disabled', 'disabled');

				/*
				 * Discover the TOKENIDX if it's available.
				 */
				var sp_err = document.getElementById('sp_err');
				if (containsParameter(g_tokenIdx))
				{
					/* Remove the warning about the script not having loaded */
					sp_err.replaceChild(document.createTextNode("Slate Permutate TOKENIDX-acquiring script loaded…"), sp_err.firstChild);
					sp_err.setAttribute('style', 'color: grey;');

					/* Inform home base of the newly generated TOKENIDX. */
					var TOKENIDX = getURLParameter(g_tokenIdx);
					if (getURLParameter('URL').indexOf('TOKENIDX%3d' + TOKENIDX) === -1)
					{
						/* %26 = &, setURLParameter doesn’t handle escaping */
						setURLParameter('URL', getURLParameter('URL') + '%26TOKENIDX%3d' + TOKENIDX);
						window.location.href = getBaseURI(window.location.href) + '?' + getURLParameters();
					}
					else
					{
						/* Report to the user that they’ve been fixed up */
						slate_permutate_input_login.setAttribute('value', 'LOG IN');
						slate_permutate_input_login.removeAttribute('disabled');
						sp_err.replaceChild(document.createTextNode('Slate Permutate has acquired WebAdvisor TOKENIDX, ready for login.'), sp_err.firstChild);
						sp_err.setAttribute('style', 'color: green;');
					}
				}
				else
				{
					sp_err.replaceChild(document.createTextNode('Slate Permutate unable to acquire TOKENIDX. You must register manually.'), sp_err.firstChild);
					sp_err.setAttribute('style', 'color: red; background: yellow;');
						alert('Unable to discover WebAdvisor TOKENIDX. You must register manually.');
				}
		}

		/*
		 * Register to run after either of getWindowHTML(),
		 * setWindowHTML(), or displayFormHTML() have been run. These are
		 * run after onload="", so they are required if we're to wait for
		 * the DOM to load...
		 */
		var funcs = ['getWindowHTML', 'setWindowHTML', 'displayFormHTML'];
		for (var i = 0; i < funcs.length; i ++)
		{
				var func = window[funcs[i]];
				window[funcs[i]] = function() {
						func();
						slate_permutate_onload();
				};
		}
})();
