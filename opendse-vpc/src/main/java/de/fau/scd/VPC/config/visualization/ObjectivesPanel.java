// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.config.visualization;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.lang.reflect.InvocationTargetException;
import java.util.Vector;
import java.util.regex.Pattern;

import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableColumn;

import org.opt4j.core.Objective;
import org.opt4j.core.config.Property;

import de.fau.scd.VPC.config.properties.ObjectiveInfo;
import de.fau.scd.VPC.config.properties.Objectives;

@SuppressWarnings("serial")
public class ObjectivesPanel
    extends
        JScrollPane
    implements
        ActionListener
      , TableModelListener
{

    public ObjectivesPanel(Property property) {
        this.property = property;
        this.objectives = (Objectives) property.getValue();

        tableModel = new DefaultTableModel(
                new Object[]{"Name", "MIN/MAX", "File", "Regex"}, 0);
        for (ObjectiveInfo e : objectives) {
            tableModel.addRow(new Object[]{
                e.getObjName()
              , e.getObjSign()
              , e.getParseFile().toString()
              , e.getParseRegex().pattern()});
        }
//      tableModel.addRow(new Object[]{"foo", "bar"});
        tableModel.addTableModelListener(this);
        table = new JTable(tableModel);

        {
            TableColumn signColumn = table.getColumnModel().getColumn(1);
            // Set up the editor for the attr. type column.
            JComboBox<Objective.Sign> comboBox = new JComboBox<Objective.Sign>();
            for (Objective.Sign objSign : Objective.Sign.values())
                comboBox.addItem(objSign);
            signColumn.setCellEditor(new DefaultCellEditor(comboBox));
        }

        this.setViewportView(table);

        {
            JPopupMenu popupMenu = new JPopupMenu();
            menuItemAdd1 = new JMenuItem("New objective");
            menuItemAdd1.addActionListener(this);
            popupMenu.add(menuItemAdd1);
            menuItemRemove1 = new JMenuItem("Remove selected objectives");
            menuItemRemove1.addActionListener(this);
            popupMenu.add(menuItemRemove1);
            // Set the popup menu for the table
            table.setComponentPopupMenu(popupMenu);
        }

        {
            JPopupMenu popupMenu = new JPopupMenu();
            menuItemAdd2 = new JMenuItem("New objective");
            menuItemAdd2.addActionListener(this);
            popupMenu.add(menuItemAdd2);
            menuItemRemove2 = new JMenuItem("Remove selected objectives");
            menuItemRemove2.addActionListener(this);
            popupMenu.add(menuItemRemove2);
            // Set the popup menu for the table
            this.setComponentPopupMenu(popupMenu);
        }

//      System.err.println(table.getMinimumSize());
//      System.err.println(table.getPreferredSize());
//      System.err.println(scrollPane.getMinimumSize());
//      System.err.println(scrollPane.getPreferredSize());
        this.setMinimumSize(new Dimension(-1, 22*4));
        this.setPreferredSize(this.getMinimumSize());

    }

    private final Property property;
    private final Objectives objectives;
    private final DefaultTableModel tableModel;
    private final JTable table;
    private final JMenuItem menuItemAdd1, menuItemAdd2;
    private final JMenuItem menuItemRemove1, menuItemRemove2;

    @Override
    public void actionPerformed(ActionEvent event) {
        JMenuItem menu = (JMenuItem) event.getSource();
        if (menu == menuItemAdd1 || menu == menuItemAdd2) {
            addNewRow();
        } else if (menu == menuItemRemove1 || menu == menuItemRemove2) {
            removeSelectedRows();
        }
    }

    private void addNewRow() {
        tableModel.addRow(new String[0]);
    }

    private void removeSelectedRows() {
        int[] selectedRows = table.getSelectedRows();

        for (int i = selectedRows.length-1; i >= 0; --i) {
            tableModel.removeRow(selectedRows[i]);
        }
    }

    @Override
    public void tableChanged(TableModelEvent e) {
//      System.err.println(e);
        objectives.clear();
        @SuppressWarnings("unchecked")
        Vector<Vector<Object>> rows = tableModel.getDataVector();
//      System.err.println(rows);
        for (Vector<Object> row : rows) {
            final String         objName    = (String) row.get(0);
            final Objective.Sign objSign    = row.get(1) != null
                ? (Objective.Sign) row.get(1)
                : Objective.Sign.MIN;
            final String         parseFile  = row.get(2) != null
                ? (String) row.get(2)
                : "";
            final String         parseRegex = row.get(3) != null
                ? (String) row.get(3)
                : "";
            if (objName != null) {
                objectives.add(new ObjectiveInfo(
                    objName
                  , objSign
                  , new File(parseFile)
                  , Pattern.compile(parseRegex)));
            }
        }
        try {
            property.setValue(objectives);
        } catch (InvocationTargetException e1) {
            e1.printStackTrace();
        }
    }

}
