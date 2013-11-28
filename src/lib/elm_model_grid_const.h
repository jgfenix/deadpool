
#ifndef ELM_MODEL_GRID_CONST_H
#define ELM_MODEL_GRID_CONST_H

/**
 * @def elm_model_grid_row_select
 * @since 1.8
 *
 * @arg row The row to select.
 *
 * Select a @p row of this grid model.
 *
 */
EAPI void elm_model_grid_row_select(Elm_Model_Grid_Row row); // XXX EINA_CONST; ?

/**
 * @def elm_model_grid_row_select
 * @since 1.8
 *
 * @param row The row coordinate of the cell.
 * @param column The column coordinate of the cell.
 * 
 * Select a cell from this grid model.
 */
EAPI void elm_model_grid_cell_select(Elm_Model_Grid_Row row, Elm_Model_Grid_Column column);

/**
 * @def elm_model_grid_columns_get
 * @since 1.8
 *
 * get a column index list from grid model
 *
 * TODO document XXX
 */
EAPI Eina_List elm_model_grid_columns_get();

/**
 * @def elm_model_grid_rows_count
 * @since 1.8
 *
 * Get the numbers of rows of this grid model.
 *
 * @return The number of rows in this grid, as integer.
 */
EAPI int elm_model_grid_rows_count();

/**
 * @def elm_model_grid_columns_count
 * @since 1.8
 *
 * Get the numbers of columns of this grid model.
 *
 * @return The number of columns in this grid, as integer.
 */
EAPI int elm_model_grid_columns_count();

/**
 * @def elm_model_grid_value_get
 * @since 1.8
 *
 * Get the value of a cell from this grid model.
 *
 * @param row The row coordinate of the cell.
 * @param column The column coordinate of the cell.
 * @return The value, as @ref Eina_Value.
 */
EAPI Eina_Value elm_model_grid_value_get(Elm_Model_Grid_Row row, Elm_Model_Grid_Column column);

/**
 * @brief EO3 Class Declaration
 */
#define ELM_MODEL_GRID_CONST_INTERFACE elm_model_grid_const         \
   , function(elm_model_grid_row_select, void, Elm_Model_Grid_Row)  \
   , function(elm_model_grid_rows_count, int)                       \
   , function(elm_model_grid_cell_select, void, Elm_Model_Grid_Row, Elm_Model_Grid_Column) /* XXX */ \
   , function(elm_model_grid_columns_get, Eina_List)  /*XXX*/       \
   , function(elm_model_grid_columns_count, int)                    \
   , function(elm_model_grid_value_get, Eina_Value, Elm_Model_Grid_Row, Elm_Model_Grid_Column)
   /* , event(elm_model_grid_row_selected) \ */
   /* , event(elm_model_grid_reordered) */

EO3_DECLARE_INTERFACE(ELM_MODEL_GRID_CONST_INTERFACE)

#endif // ELM_MODEL_GRID_CONST_H
