/*
 * Assumes that WebAdvisor_scripts.js for WebAdvisor-2.x is loaded,
 * displayFormHTML() or something was called and thus
 * readURLParameters() was called. We attempt to extract TOKENIDX and
 * asynchronously inform slate_permutate about it. We currently assume
 * we're on a login form too.
 */

var slate_permutate_input_login;

(function() {
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
				if (containsParameter(g_tokenIdx))
				{
						var TOKENIDX = getURLParameter(g_tokenIdx);
						var myscript = document.createElement('script');
						myscript.setAttribute('type', 'text/javascript');
						myscript.setAttribute('src', decodeURIComponent(getURLParameter('SP_CALLBACK')) + 'callback=slate_permutate_token_callback&TOKENIDX=' + TOKENIDX);
						document.getElementsByTagName('head').item(0).appendChild(myscript);
				}
				else
				{
						alert('Unable to discover TOKENIDX. You must register manually.');
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

function slate_permutate_token_callback(result)
{
		if (result)
		{
				slate_permutate_input_login.setAttribute('value', 'LOG IN');
				slate_permutate_input_login.removeAttribute('disabled');
		}
}
