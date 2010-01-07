#include "lib.h"

#include "sieve-common.h"
#include "sieve-ast.h"
#include "sieve-binary.h"
#include "sieve-code.h"
#include "sieve-commands.h"
#include "sieve-validator.h"
#include "sieve-generator.h"
#include "sieve-interpreter.h"
#include "sieve-dump.h"

#include "sieve-ext-variables.h"

#include "ext-extdata-common.h"
#include "ext-extdata-variables.h"

bool vnspc_sieve_extdata_validate
	(struct sieve_validator *valdtr, const struct sieve_variables_namespace *nspc,
		struct sieve_ast_argument *arg, struct sieve_command *cmd,
		ARRAY_TYPE(sieve_variable_name) *var_name, void **var_data,
		bool assignment);
bool vnspc_sieve_extdata_generate
	(const struct sieve_codegen_env *cgenv,
		const struct sieve_variables_namespace *nspc,
		struct sieve_ast_argument *arg, struct sieve_command *cmd, void *var_data);
bool vnspc_sieve_extdata_read_variable
	(const struct sieve_runtime_env *renv,
		const struct sieve_variables_namespace *nspc, sieve_size_t *address,
		string_t **str_r);
bool vnspc_sieve_extdata_dump_variable
	(const struct sieve_dumptime_env *denv,
		const struct sieve_variables_namespace *nspc, sieve_size_t *address,
		const char *field_name);

static const struct sieve_variables_namespace_def extdata_namespace = {
	SIEVE_OBJECT("extdata", &extdata_namespace_operand, 0),
	vnspc_sieve_extdata_validate,
	vnspc_sieve_extdata_generate,
	vnspc_sieve_extdata_read_variable,
	vnspc_sieve_extdata_dump_variable
};

bool vnspc_sieve_extdata_validate
(struct sieve_validator *valdtr, 
	const struct sieve_variables_namespace *nspc ATTR_UNUSED,
	struct sieve_ast_argument *arg, struct sieve_command *cmd ATTR_UNUSED,
	ARRAY_TYPE(sieve_variable_name) *var_name, void **var_data,
	bool assignment)
{
	struct sieve_ast *ast = arg->ast;
	const struct sieve_variable_name *name_element;
	const char *variable;

	/* Check variable name */

	if ( array_count(var_name) != 2 ) {
		sieve_argument_validate_error(valdtr, arg,
			"extdata: invalid variable name within extdata namespace: "
			"encountered sub-namespace");
		return FALSE;
 	}

	name_element = array_idx(var_name, 1);
	if ( name_element->num_variable >= 0 ) {
		sieve_argument_validate_error(valdtr, arg,
			"extdata: invalid variable name within extdata namespace 'extdata.%d': "
			"encountered numeric variable name", name_element->num_variable);
		return FALSE;
	}

	variable = str_c(name_element->identifier);

	if ( assignment ) {
		sieve_argument_validate_error(valdtr, arg,
			"extdata: cannot assign to extdata variable extdata.%s", variable);
		return FALSE;
	}

	*var_data = (void *) p_strdup(sieve_ast_pool(ast), variable);

	return TRUE;
}

bool vnspc_sieve_extdata_generate
(const struct sieve_codegen_env *cgenv,
	const struct sieve_variables_namespace *nspc,
	struct sieve_ast_argument *arg ATTR_UNUSED,
	struct sieve_command *cmd ATTR_UNUSED, void *var_data)
{
	const struct sieve_extension *this_ext = SIEVE_OBJECT_EXTENSION(nspc);	
	const char *variable = (const char *) var_data;
	struct ext_sieve_extdata_context *ext_data;

	if ( this_ext == NULL )
		return FALSE;

	ext_data = (struct ext_sieve_extdata_context *) this_ext->context;

	sieve_variables_opr_namespace_variable_emit
		(cgenv->sbin, ext_data->var_ext, this_ext, &extdata_namespace);
	sieve_binary_emit_cstring(cgenv->sbin, variable);

	return TRUE;
}

bool vnspc_sieve_extdata_read_variable
(const struct sieve_runtime_env *renv,
	const struct sieve_variables_namespace *nspc, sieve_size_t *address,
	string_t **str_r)
{
	const struct sieve_extension *this_ext = SIEVE_OBJECT_EXTENSION(nspc);	
	string_t *var_name;
	const char *ext_value;

	if ( !sieve_binary_read_string(renv->sbin, address, &var_name) )
		return FALSE;

	if ( str_r !=  NULL ) {
		ext_value=ext_extdata_get_value(renv, this_ext, str_c(var_name));
		if ( ext_value == NULL ) {
			*str_r = t_str_new_const("", 0);
			return TRUE;
		}

		*str_r = t_str_new_const(ext_value, strlen(ext_value));
	}
	return TRUE;
}

bool vnspc_sieve_extdata_dump_variable
(const struct sieve_dumptime_env *denv,
	const struct sieve_variables_namespace *nspc ATTR_UNUSED, 
	sieve_size_t *address, const char *field_name)
{
	string_t *var_name;

	if ( !sieve_binary_read_string(denv->sbin, address, &var_name) )
		return FALSE;

	if ( field_name != NULL )
		sieve_code_dumpf(denv, "%s: VAR ${extdata.%s}",
			field_name, str_c(var_name));
	else
		sieve_code_dumpf(denv, "VAR ${extdata.%s}",
			str_c(var_name));

	return TRUE;
}

/*
 * Namespace registration
 */

static const struct sieve_extension_objects extdata_namespaces =
	SIEVE_VARIABLES_DEFINE_NAMESPACE(extdata_namespace);

const struct sieve_operand_def xtdata_namespace_operand = {
	"extdata-namespace",
	&extdata_extension,
	0,
	&sieve_variables_namespace_operand_class,
	&extdata_namespaces
};

void ext_extdata_variables_init
(const struct sieve_extension *this_ext, struct sieve_validator *valdtr)
{
	struct ext_extdata_context *ext_data =
		(struct ext_extdata_context *) this_ext->context;

	sieve_variables_namespace_register
		(ext_data->var_ext, valdtr, this_ext, &extdata_namespace);
}
