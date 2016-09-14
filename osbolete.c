
/** ---------------------------------------------------------------------------
 * 
 * ---------------------------------------------------------------------------- */
uint16_t valid_filename_char (uint8_t c)
{
	if ((c >= '0' && c <= '9')
		|| (c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z')
		|| (c == '_')
		|| (c == '+')
		|| (c == '.')
		|| (c == '~'))
		return 1;
	else
		return 0;
}

/* ----------------------------------------------------------------------------
 * 
 * ---------------------------------------------------------------------------- */
uint8_t * valid_filename (void)
{
	// SDL - I wasn't sure if this functionality existed above, so I figured i'd put it here
	uint8_t * ret = txtpos;
	error_code = 0;
	// make sure there are no quotes or spaces, search for valid characters
	// while (*txtpos == SPACE || *txtpos == TAB || *txtpos == SQUOTE || *txtpos == DQUOTE) txtpos++;
	while (!valid_filename_char (*txtpos))
		txtpos++;
	ret = txtpos;
	if (*ret == '\0') {
		error_code = 1;
		return ret;
	}
	// find the next invalid char
	txtpos++;
	while (valid_filename_char (*txtpos))
		txtpos++;
	if (txtpos != ret)
		*txtpos = '\0';
	// set error code if no string
	if (*ret == '\0')
		error_code = 1;
	return ret;
}

